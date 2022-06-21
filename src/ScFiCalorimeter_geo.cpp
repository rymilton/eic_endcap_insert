#include <iostream>
#include <cmath>
#include <algorithm>

#include <XML/Helper.h>
#include "DD4hep/DetFactoryHelper.h"

using namespace dd4hep;

// helper function to get x, y, z if defined in a xml component
template<class XmlComp>
Position get_xml_xyz(const XmlComp &comp, dd4hep::xml::Strng_t name)
{
  Position pos(0., 0., 0.);
  if (comp.hasChild(name)) {
    auto child = comp.child(name);
    pos.SetX(dd4hep::getAttrOrDefault<double>(child, _Unicode(x), 0.));
    pos.SetY(dd4hep::getAttrOrDefault<double>(child, _Unicode(y), 0.));
    pos.SetZ(dd4hep::getAttrOrDefault<double>(child, _Unicode(z), 0.));
  }
  return pos;
}

// main
static Ref_t create_detector(Detector &desc, xml::Handle_t handle, SensitiveDetector sens)
{
  xml::DetElement detElem = handle;
  std::string detName = detElem.nameStr();
  int detID = detElem.id();
  DetElement det(detName, detID);
  sens.setType("calorimeter");
  auto dim = detElem.dimensions();
  auto rmin = dim.rmin();
  auto rmax = dim.rmax();
  auto length = dim.length();
  auto phimin = dd4hep::getAttrOrDefault<double>(dim, _Unicode(phimin), 0.);
  auto phimax = dd4hep::getAttrOrDefault<double>(dim, _Unicode(phimax), 2.*M_PI);
  // envelope
  Tube envShape(rmin, rmax, length/2., phimin, phimax);
  Volume envVol(detName + "_envelope", envShape, desc.material("Air"));
  envVol.setVisAttributes(desc.visAttributes(detElem.visStr()));

  const xml::Component &insert_xml = detElem.child(_Unicode(insert));
  const auto insert_sx = insert_xml.attr<double>(_Unicode(sx));
  const auto insert_sy = insert_xml.attr<double>(_Unicode(sy));
  const auto insert_sz = insert_xml.attr<double>(_Unicode(sz));

  const xml::Component &block_xml = detElem.child(_Unicode(block));
  const auto pipe_sr = block_xml.attr<double>(_Unicode(pipe_r));
  const auto block_sr = block_xml.attr<double>(_Unicode(radius));
  const auto block_sz = block_xml.attr<double>(_Unicode(length));
  auto modMat = desc.material(block_xml.attr<std::string>(_Unicode(material)));
  // calorimeter block z-offsets (as blocks are shorter than the volume length)
  const double block_offset = -length/2. + block_sz/2.;

  Tube calShape(pipe_sr, block_sr, block_sz/2.);
  Box insertShape(insert_sx/2., insert_sy/2., insert_sz/2.);
  SubtractionSolid blockShape(calShape, insertShape);
  Volume blockVol("block_vol", blockShape, modMat);
  blockVol.setSensitiveDetector(sens);
  auto blockPV = envVol.placeVolume(blockVol, Position{0, 0, block_offset});
  blockPV.addPhysVolID("block", 1);
  desc.add(Constant(detName + "_NModules", std::to_string(1)));

  // detector position and rotation
  auto pos = get_xml_xyz(detElem, _Unicode(position));
  auto rot = get_xml_xyz(detElem, _Unicode(rotation));
  Volume motherVol = desc.pickMotherVolume(det);
  Transform3D tr = Translation3D(pos.x(), pos.y(), pos.z()) * RotationZYX(rot.z(), rot.y(), rot.x());
  PlacedVolume envPV = motherVol.placeVolume(envVol, tr);
  envPV.addPhysVolID("system", detID);
  det.setPlacement(envPV);
  return det;
}

DECLARE_DETELEMENT(ScFiCalorimeter, create_detector)
