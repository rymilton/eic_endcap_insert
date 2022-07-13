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
  double     rmin       = dim.rmin();
  double     rmax       = dim.rmax();
  double     length     = dim.z();

  xml_dim_t  pos        = x_det.position();
  const double     x    = pos.x();
  double     z          = pos.z();

  Material   Vacuum     = desc.material("Vacuum");
  
  // Getting insert dimensions
  const xml::Component &insert_xml = x_det.child(_Unicode(insert));
  xml_dim_t  insert_dim        = insert_xml.dimensions();
  xml_dim_t  insert_local_pos  = insert_xml.position();


  Tube envelope(rmin, rmax, length / 2.0);
  Box insert(insert_dim.x() / 2., insert_dim.y() / 2., length / 2.);
  SubtractionSolid envelope_with_inserthole(
    envelope,
    insert,
    Position(insert_local_pos.x(), insert_local_pos.y(), 0.)
  );

  Volume envelopeVol(detName+"_envelope", envelope_with_inserthole, Vacuum);
  // Not set yet
  envelopeVol.setVisAttributes(desc.visAttributes(x_det.visStr()));

  PlacedVolume pv;

  int layer_num = 1;
  // Read layers
  for(xml_coll_t c(x_det,_U(layer)); c; ++c)
  {
    xml_comp_t x_layer = c;
    int repeat = x_layer.repeat();
    double layer_thickness = x_layer.thickness();

    // Loop over repeat
    for(int i = 0; i < repeat; i++) {
      double zlayer = z; // zlayer increases by layer_thickness over each loop
      string layer_name = detName + _toString(layer_num, "_layer%d");
      Tube layer(rmin, rmax, layer_thickness / 2.);

      // Removing insert shape from each layer
      Box layer_insert(
        insert_dim.x() / 2.,
        insert_dim.y() / 2.,
        layer_thickness / 2.
      );
      SubtractionSolid layer_with_inserthole(
        layer,
        layer_insert,
        Position(insert_local_pos.x(), insert_local_pos.y(), 0.)
      );
      Volume layer_vol(layer_name, layer_with_inserthole, Vacuum);

      int slice_num = 1;
      // Loop over slices
      for(xml_coll_t l(x_layer,_U(slice)); l; ++l) {

        xml_comp_t x_slice = l;
        double slice_thickness = x_slice.thickness();
        string slice_name = layer_name + _toString(slice_num,"slice%d");
        Material slice_mat = desc.material(x_slice.materialStr());

        Tube slice(rmin, rmax, slice_thickness/2.0);
        Box slice_insert(
          insert_dim.x() / 2.,
          insert_dim.y() / 2.,
          slice_thickness / 2.0
        );

        // Removing insert shape from each slice
        SubtractionSolid slice_with_inserthole(
          slice,
          slice_insert,
          Position(insert_local_pos.x(), insert_local_pos.y(), 0.)
        );
        Volume slice_vol (slice_name, slice_with_inserthole, slice_mat);

        if(x_slice.isSensitive()) {
          sens.setType("calorimeter");
          slice_vol.setSensitiveDetector(sens);
        }

        slice_vol.setAttributes(
          desc,x_slice.regionStr(),
          x_slice.limitsStr(),
          x_slice.visStr()
        );
        pv = layer_vol.placeVolume(
          slice_vol,
          Transform3D(
            RotationZYX(0, 0, 0),
            Position(0., 0., z - zlayer - layer_thickness/2. + slice_thickness/2.)
          )
        );
        pv.addPhysVolID("slice", slice_num);
        z += slice_thickness;
        ++slice_num;
      }

      string layer_vis = dd4hep::getAttrOrDefault(
        x_layer,
        _Unicode(vis),
        "InvisibleWithDaughters"
      );
      layer_vol.setAttributes(
        desc,
        x_layer.regionStr(),
        x_layer.limitsStr(),
        layer_vis
      );
      pv = envelopeVol.placeVolume(
        layer_vol,
        Transform3D(
          RotationZYX(0, 0, 0),
          Position(0., 0. , zlayer - pos.z() - length/2. + layer_thickness/2.)
        )
      );
      pv.addPhysVolID("layer", layer_num);
      layer_num++;
    }
  }

  DetElement   det(detName, detID);  
  Volume motherVol = desc.pickMotherVolume(det);
  
  auto tr = Transform3D(Position(pos.x(), pos.y(), pos.z() + length/2.));

  PlacedVolume phv = motherVol.placeVolume(envelopeVol, tr);
  phv.addPhysVolID("system", detID);
  det.setPlacement(phv);

  return det;
}
DECLARE_DETELEMENT(pEndcap, createDetector)