#include "DD4hep/Detector.h"
#include "DDG4/Geant4Data.h"
#include "DDRec/CellIDPositionConverter.h"
#include "DDRec/SurfaceManager.h"
#include "DDRec/Surface.h"


#include "fmt/core.h"


/** Example using dd4hep
 */
void example_dd4hep(const char* fname = "test_tracker_disc.root"){

  using namespace ROOT::Math;

  // -------------------------
  // Get the DD4hep instance
  // Load the compact XML file
  // Initialize the position converter tool
  dd4hep::Detector& detector = dd4hep::Detector::getInstance();
  detector.fromCompact("athena.xml");
  dd4hep::rec::CellIDPositionConverter cellid_converter(detector);

  fmt::print("Detector Types:\n");
  for(const auto&  dtype : detector.detectorTypes() ) {
    fmt::print("  {}\n", dtype); 
  }

  fmt::print("\n");
  fmt::print("All detector subsystem names:\n");
  for(const auto&  d : detector.detectors() ) {
    fmt::print("  {}\n", d.first);
  }

}
