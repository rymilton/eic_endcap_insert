#include "DDRec/Surface.h"
#include "DDRec/DetectorData.h"
#include "DD4hep/OpticalSurfaces.h"
#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/Printout.h"
#include <XML/Helper.h>
//////////////////////////////////
// Electron Endcap GEM Tracking
//////////////////////////////////

using namespace std;
using namespace dd4hep;

static Ref_t createDetector(Detector& desc, xml_h e, SensitiveDetector sens)
{
  xml_det_t  x_det      = e;
  string     detName    = x_det.nameStr();
  int        detID      = x_det.id();

  xml_dim_t  dim        = x_det.dimensions();
  double     RIn        = dim.rmin();
  double     ROut       = dim.rmax();
  double     SizeZ      = dim.length();
  xml_dim_t  pos        = x_det.position();

  Material   Vacuum     = desc.material("Vacuum");

  // Create Global Volume 
  Tube ce_GEM_GVol_Solid(RIn, ROut, SizeZ / 2.0, 0., 360.0 * deg);
  Volume detVol("ce_GEM_GVol_Logic", ce_GEM_GVol_Solid, Vacuum);
  detVol.setVisAttributes(desc.visAttributes(x_det.visStr()));

  // Construct Layers
  xml_comp_t x_layer = x_det.child(_U(layer));
  const int repeat   = x_layer.repeat();
  
  xml_comp_t x_slice = x_layer.child(_U(slice));
  Material slice_mat = desc.material(x_slice.materialStr());
  double layerSizeZ = x_slice.thickness();
  double layerRIn;
  double layerROut;
  double layerPosZ;

  // Loop over layers
  for(int i = 0; i < repeat; i++) {
    layerRIn  = RIn + 1.0 * cm + ((double)i * 0.5) * cm;
    layerROut = ROut;//RIn + ((double)i * 0.5) * cm;
    layerPosZ = SizeZ / 2.0 - 5.0 * cm - ((double)i * 3.0) * cm;
    layerSizeZ = 1.0 * cm;

    string logic_layer_name = detName + _toString(i, "_Logic_lay_%d");
    Volume layerVol(logic_layer_name,Tube(layerRIn, layerROut, layerSizeZ / 2.0, 0.0, 360.0 * deg), slice_mat);
    layerVol.setVisAttributes(desc,x_layer.visStr());
    sens.setType("tracker");
    layerVol.setSensitiveDetector(sens);

    Position     layer_pos = Position(0.0, 0.0, layerPosZ);
    PlacedVolume layerPV = detVol.placeVolume(layerVol, layer_pos);
    layerPV.addPhysVolID("layer", i+1);
  }

  DetElement   det(detName, detID);
  Volume       motherVol = desc.pickMotherVolume(det);
  Transform3D  tr(RotationZYX(0.0, 0.0, 0.0), Position(pos.x(), pos.y(), pos.z()));
  PlacedVolume detPV = motherVol.placeVolume(detVol, tr);
  detPV.addPhysVolID("system", detID);
  det.setPlacement(detPV);
  return det;
}

DECLARE_DETELEMENT(ce_GEM, createDetector)
