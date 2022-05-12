#include "DDRec/Surface.h"
#include "DDRec/DetectorData.h"
#include "DD4hep/OpticalSurfaces.h"
#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/Printout.h"
#include <XML/Helper.h>
//////////////////////////////////////////////////
// Far Forward Ion Zero Degree Calorimeter - Ecal
//////////////////////////////////////////////////

using namespace std;
using namespace dd4hep;

static Ref_t createDetector(Detector& desc, xml_h e, SensitiveDetector sens)
{
  xml_det_t  x_det      = e;
  string     detName    = x_det.nameStr();
  int        detID      = x_det.id();

  xml_dim_t  dim        = x_det.dimensions();
  double     width      = dim.x();
  double     height      = dim.y();
  double     length  =    dim.z();
  
  xml_dim_t  pos        = x_det.position();
  double     z          = pos.z();
  xml_dim_t  rot        = x_det.rotation();

  Material   Vacuum     = desc.material("Vacuum");

  // Getting insert information
  const xml::Component &insert_xml = x_det.child(_Unicode(insert));
  const double insert_width = dd4hep::getAttrOrDefault<double>(insert_xml, _Unicode(width), 56.4*cm);
  const double insert_height = dd4hep::getAttrOrDefault<double>(insert_xml, _Unicode(height), 59.7485*cm);
  const double insert_length = dd4hep::getAttrOrDefault<double>(insert_xml, _Unicode(length), 117.3*cm);

  // Additional space between the hcal and the insert
  // Need to adjust these according to Oleg's specs
  const double hcal_insert_x_gap = 1.8 * cm;
  const double hcal_insert_y_gap = 0.0841 * cm;

  // Create Envelope Volume
  Box envShape(width/2., height/2., length/2.);
  Box insertCutout((insert_width + hcal_insert_x_gap) / 2., (insert_height + hcal_insert_y_gap)/ 2., insert_length / 2.);
  SubtractionSolid evelopeAroundInsert(envShape, insertCutout);
  Volume envelopeVolume(detName + "_envelope", evelopeAroundInsert, Vacuum);
  envelopeVolume.setVisAttributes(desc.visAttributes(x_det.visStr()));

  PlacedVolume pv;

  int layer_num = 1;
  for(xml_coll_t c(x_det,_U(layer)); c; ++c) 
  {
    xml_comp_t x_layer = c;
    int repeat = x_layer.repeat();  
    double layer_thickness = 0;
    
    for(xml_coll_t l(x_layer,_U(slice)); l; ++l)
      layer_thickness += xml_comp_t(l).thickness();

    // Loop over repeat
    for(int i = 0; i < repeat; i++) {
      double zlayer = z; // zlayer increases by layer_thickness over each layer repeat
      string layer_name = detName + _toString(layer_num, "_layer%d");

      Box layer(width / 2., height / 2., layer_thickness / 2.);
      Box layer_insertCutout((insert_width + hcal_insert_x_gap) / 2., (insert_height + hcal_insert_y_gap)/ 2., layer_thickness / 2.);
      SubtractionSolid layer_with_insert(layer, layer_insertCutout);
      Volume layer_vol(layer_name, layer_with_insert, Vacuum);

      int slice_num = 1;
      // Loop over slices
      for(xml_coll_t l(x_layer,_U(slice)); l; ++l) {
        xml_comp_t x_slice = l;
        double slice_thickness = x_slice.thickness();
        string slice_name = layer_name + _toString(slice_num,"slice%d");
        Material slice_mat = desc.material(x_slice.materialStr());		

        Box slice(width/2.0, height/2.0, slice_thickness/2.0);
        Box   slice_insertCutout((insert_width + hcal_insert_x_gap) / 2., (insert_height + hcal_insert_y_gap)/ 2., slice_thickness / 2.);
        SubtractionSolid slice_with_insert(slice, slice_insertCutout);
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
      pv = envelopeVolume.placeVolume(
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
  Volume       motherVol = desc.pickMotherVolume(det);
  
  const xml::Component &beampipe_region_xml = x_det.child(_Unicode(beampipe_region));
  const auto crossing_angle = dd4hep::getAttrOrDefault<double>(beampipe_region_xml, _Unicode(beampipe_crossing_angle), -0.025*radian);

  auto tr = Transform3D(RotationY(crossing_angle)) *
              Transform3D(Position(pos.x(), pos.y(), pos.z()+length/2.0));
  
  PlacedVolume detPV = motherVol.placeVolume(envelopeVolume, tr);
  detPV.addPhysVolID("system", detID);
  det.setPlacement(detPV);
  return det;
}
DECLARE_DETELEMENT(HcalEndcapPWithInsert, createDetector)