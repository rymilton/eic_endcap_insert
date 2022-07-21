#include "DDRec/Surface.h"
#include "DDRec/DetectorData.h"
#include "DD4hep/OpticalSurfaces.h"
#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/Printout.h"
#include <XML/Helper.h>
#include <XML/Layering.h>

using namespace std;
using namespace dd4hep;

static Ref_t createDetector(Detector& desc, xml_h e, SensitiveDetector sens)
{
  xml_det_t  x_det      = e;
  string     detName    = x_det.nameStr();
  int detID             = x_det.id();

  xml_dim_t  dim        = x_det.dimensions();
  double     width      = dim.x();
  double     height     = dim.y();
  double     length     = dim.z();

  xml_dim_t  pos        = x_det.position();
  const double     x    = pos.x();
  xml_dim_t  rot        = x_det.rotation();

  Material   Vacuum     = desc.material("Vacuum");
  
  // Getting beampipe dimensions
  const xml::Component &beampipe_region_xml = x_det.child(
    _Unicode(beampipe_region)
  );
  const auto beampipe_radius_initial = dd4hep::getAttrOrDefault<double>(
    beampipe_region_xml,
    _Unicode(initial_beampipe_radius),
    15.*cm
  );
  const auto beampipe_radius_final = dd4hep::getAttrOrDefault<double>(
    beampipe_region_xml,
    _Unicode(final_beampipe_radius),
    17.25*cm
  );

  // Getting thickness of final layer for hole radius function
  auto final_layer_thickness = dd4hep::getAttrOrDefault<double>(
    x_det,
    _Unicode(final_layer_thickness),
    1.61*cm
  );

  // Function that returns a linearly interpolated beampipe radius at a given z
  auto get_beampipe_radius = [
    beampipe_radius_initial, 
    beampipe_radius_final, 
    length,
    final_layer_thickness
  ]
  (double z_pos) 
  {
    /*
      Treats the beginning of the insert as z = 0
      The radius is beampipe_radius_initial at z = 0
      The radius is beampipe_radius_final at the beginning of the last layer, 
        i.e. z = length-final_layer_thickness
    */
    double slope = (beampipe_radius_final != beampipe_radius_initial) ? 
                    (beampipe_radius_final - beampipe_radius_initial) /
                    (length - final_layer_thickness) : 
                    0.;
    return slope * z_pos + beampipe_radius_initial;
  };
  Box envelope(width / 2.0, height / 2.0, length / 2.0);

  // Hole x-position in insert coordinate system
  const double initial_hole_x = -x - 7.7 * cm;
  // Keeps track of the hole's x location for each layer
  double hole_x_tracker = initial_hole_x;
  // Keeps track of the z location as we move longiduinally through detector
  // here, z = 0 is the front of detector
  double z_tracker = 0.;
  // Cutting out the beampipe shape from the envelope
  for(xml_coll_t c(x_det,_U(layer)); c; ++c) 
  {
    xml_comp_t x_layer = c;
    int repeat = x_layer.repeat();
    double layer_thickness = x_layer.thickness();
    for (int ilayer = 0; ilayer < repeat; ilayer++)
    {
      const double beampipe_radius = get_beampipe_radius(z_tracker);
      Tube layer_beampipe(0., beampipe_radius, layer_thickness / 2.0);
      
      /*

        X-Position:
        The hole starts at x = -7.7 cm with respect to global coordinate system.
        SubtractionSolid Position is with respect to envelope coordinate system.
        Need to shift back to global with the "-x" in initial_hole_x
        The hole shifts by -.0569 cm with each layer

        Z-Position:
        -length / 2. is front of insert,
        +z_tracker goes to the front of each layer,
        +layer_thickness / 2. is the half-length of a layer
          (i.e. where to put the cutout shape)
      */
      SubtractionSolid envelope_with_insert(
        envelope, 
        layer_beampipe, 
        Position(
          hole_x_tracker,
          0., 
          (-length + layer_thickness)/2 + z_tracker 
        )
      );
      // Removing the beampipe shape layer by layer
      envelope = envelope_with_insert;

      z_tracker += layer_thickness; // Moving hole along z
      hole_x_tracker -= 0.0569; // Moving hole along x
    }
  }
  Volume envelopeVol(detName+"_envelope", envelope, Vacuum); 
  // Not set yet
  envelopeVol.setVisAttributes(desc.visAttributes(x_det.visStr()));

  PlacedVolume pv;
  // Resetting trackers for building the layers
  hole_x_tracker = initial_hole_x;
  z_tracker = 0.;
  int layer_num = 1;
  // Read layers
  for(xml_coll_t c(x_det,_U(layer)); c; ++c) 
  {
    xml_comp_t x_layer = c;
    int repeat = x_layer.repeat();  
    double layer_thickness = x_layer.thickness();
    // Loop over repeat
    for(int i = 0; i < repeat; i++) {
      double zlayer = z_tracker; // z-position of front of each layer
      string layer_name = detName + _toString(layer_num, "_layer%d");

      Box layer(width / 2., height / 2., layer_thickness / 2.);
      // The cutout shape should change at the start of each layer
      const double beampipe_radius = get_beampipe_radius(z_tracker);

      // Removing beampipe shape from each layer
      Tube layer_beampipe(0., beampipe_radius, layer_thickness / 2.);
      SubtractionSolid layer_with_insert(
        layer,
        layer_beampipe,
        Position(hole_x_tracker, 0., 0.)
      );
      Volume layer_vol(layer_name, layer_with_insert, Vacuum);

      int slice_num = 1;
      // Loop over slices
      for(xml_coll_t l(x_layer,_U(slice)); l; ++l) {

        xml_comp_t x_slice = l;
        double slice_thickness = x_slice.thickness();
        string slice_name = layer_name + _toString(slice_num,"slice%d");
        Material slice_mat = desc.material(x_slice.materialStr());		

        // Each slice within a layer has the same cutout radius
        Box slice(width/2.0, height/2.0, slice_thickness/2.0);
        Tube slice_beampipe(0., beampipe_radius, slice_thickness / 2.0);
        SubtractionSolid slice_with_insert(
            slice,
            slice_beampipe,
            Position(hole_x_tracker, 0., 0.));
        Volume slice_vol (slice_name, slice_with_insert, slice_mat);
        
        if(x_slice.isSensitive()) {
          sens.setType("calorimeter");
          slice_vol.setSensitiveDetector(sens);
        }
        
        slice_vol.setAttributes(
          desc,x_slice.regionStr(),
          x_slice.limitsStr(),
          x_slice.visStr()
        );
        // Placing slice within layer
        // z_tracker - zlayer - layer_thickness/2. gives the front of each already placed slice
        // + slice_thickness/2. is the position of the slice about to be placed
        pv = layer_vol.placeVolume(
            slice_vol,
            Transform3D(
                RotationZYX(0, 0, 0),
                Position(
                  0.,
                  0.,
                  z_tracker - zlayer - layer_thickness / 2. + slice_thickness / 2.
                )
              )
        );
        pv.addPhysVolID("slice", slice_num);
        z_tracker += slice_thickness;
        ++slice_num;
      }
      string layer_vis = dd4hep::getAttrOrDefault(
          x_layer,
          _Unicode(vis),
          "InvisibleWithDaughters");
      layer_vol.setAttributes(
        desc,
        x_layer.regionStr(),
        x_layer.limitsStr(), 
        layer_vis
      );
      // Placing each layer inside the envelope volume
      // -length/2. is front of detector in global coordinate system
      // + (z_tracker - layer_thickness) goes to the front of each layer
      // + layer_thickness/2. places layer in correct spot
      pv = envelopeVol.placeVolume(
        layer_vol,
        Transform3D(
          RotationZYX(0, 0, 0),
          Position(
            0.,
            0.,
            -length/2.  + (z_tracker - layer_thickness) + layer_thickness/2.
          )
        )
      );
      pv.addPhysVolID("layer", layer_num);
      layer_num++;
      hole_x_tracker -= 0.0569;
    }
  }

  DetElement   det(detName, detID);  
  Volume motherVol = desc.pickMotherVolume(det);
  
  // Placing insert in world volume
  auto tr = Transform3D(Position(pos.x(), pos.y(), pos.z() + length/2.));

  PlacedVolume phv = motherVol.placeVolume(envelopeVol, tr);
  phv.addPhysVolID("system", detID);
  det.setPlacement(phv);

  return det;
}
DECLARE_DETELEMENT(pEndcapInsert, createDetector)