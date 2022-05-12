//==========================================================================
//  AIDA Detector description implementation
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
// Author     : M.Frank
//
//==========================================================================
//
// Specialized generic detector constructor
//
//==========================================================================
//
// Implementation of the Sci Fiber geometry: M. Żurek 07/19/2021
#include "DD4hep/DetFactoryHelper.h"
#include "XML/Layering.h"
#include "Math/Point2D.h"
#include "TGeoPolygon.h"
#include "TMath.h"

using namespace std;
using namespace dd4hep;
using namespace dd4hep::detail;

typedef ROOT::Math::XYPoint Point;

// Fill fiber lattice into trapezoid starting from position (0,0) in x-z coordinate system
vector<vector<Point>> fiberPositions(double radius, double x_spacing, double z_spacing, double x, double z, double phi, double spacing_tol = 1e-2) {
  // z_spacing - distance between fiber layers in z
  // x_spacing - distance between fiber centers in x
  // x - half-length of the shorter (bottom) base of the trapezoid
  // z - height of the trapezoid
  // phi - angle between z and trapezoid arm

  vector<vector<Point>> positions;
  int z_layers = floor((z/2-radius-spacing_tol)/z_spacing); // number of layers that fit in z/2

  double z_pos = 0.;
  double x_pos = 0.;

  for(int l = -z_layers; l < z_layers+1; l++) {
    vector<Point> xline;
    z_pos = l*z_spacing;
    double x_max = x + (z/2. + z_pos)*tan(phi) - spacing_tol; // calculate max x at particular z_pos
    (l % 2 == 0) ? x_pos = 0. : x_pos = x_spacing/2; // account for spacing/2 shift

    while(x_pos < (x_max - radius)) {
      xline.push_back(Point(x_pos, z_pos));
      if(x_pos != 0.) xline.push_back(Point(-x_pos, z_pos)); // using symmetry around x=0
      x_pos += x_spacing;
    }
    // Sort fiber IDs for a better organization
    sort(xline.begin(), xline.end(), [](const Point &p1, const Point &p2) {
        return p1.x() < p2.x();
      });
    positions.emplace_back(std::move(xline));
  }
  return positions;
}

// Calculate number of divisions for the readout grid for the fiber layers
std::pair<int, int> getNdivisions(double x, double z, double dx, double dz) {
  // x and z defined as in vector<Point> fiberPositions
  // dx, dz - size of the grid in x and z we want to get close to with the polygons
  // See also descripltion when the function is called

  double SiPMsize = 13.0*mm;
  double grid_min = SiPMsize + 3.0*mm;

  if(dz < grid_min) {
    dz = grid_min;
  }

  if(dx < grid_min) {
    dx = grid_min;
  }

  int nfit_cells_z = floor(z/dz);
  int n_cells_z = nfit_cells_z;

  if(nfit_cells_z == 0) n_cells_z++;

  int nfit_cells_x = floor((2*x)/dx);
  int n_cells_x = nfit_cells_x;

  if(nfit_cells_x == 0) n_cells_x++;

  return std::make_pair(n_cells_x, n_cells_z);

}

// Calculate dimensions of the polygonal grid in the cartesian coordinate system x-z
vector< tuple<int, Point, Point, Point, Point> > gridPoints(int div_x, int div_z, double x, double z, double phi) {
  // x, z and phi defined as in vector<Point> fiberPositions
  // div_x, div_z - number of divisions in x and z
  double dz = z/div_z;

  std::vector<std::tuple<int, Point, Point, Point, Point>> points;

  for(int iz = 0; iz < div_z + 1; iz++){
    for(int ix = 0; ix < div_x + 1; ix++){
      double A_z = -z/2 + iz*dz;
      double B_z = -z/2 + (iz+1)*dz;

      double len_x_for_z = 2*(x+iz*dz*tan(phi));
      double len_x_for_z_plus_1 = 2*(x + (iz+1)*dz*tan(phi));

      double dx_for_z = len_x_for_z/div_x;
      double dx_for_z_plus_1 = len_x_for_z_plus_1/div_x;

      double A_x = -len_x_for_z/2. + ix*dx_for_z;
      double B_x = -len_x_for_z_plus_1/2. + ix*dx_for_z_plus_1;

      double C_z = B_z;
      double D_z = A_z;
      double C_x = B_x + dx_for_z_plus_1;
      double D_x = A_x + dx_for_z;

      int id = ix + div_x * iz;

      auto A = Point(A_x, A_z);
      auto B = Point(B_x, B_z);
      auto C = Point(C_x, C_z);
      auto D = Point(D_x, D_z);

      // vertex points filled in the clock-wise direction
      points.push_back(make_tuple(id, A, B, C, D));

    }
  }

  return points;

}

// Create detector
static Ref_t create_detector(Detector& description, xml_h e, SensitiveDetector sens)  {
  static double tolerance = 0e0;
  Layering      layering (e);
  xml_det_t     x_det     = e;
  Material      air       = description.air();
  int           det_id    = x_det.id();
  string        det_name  = x_det.nameStr();
  xml_comp_t    x_staves  = x_det.staves();
  xml_comp_t    x_dim     = x_det.dimensions();
  int           nsides    = x_dim.numsides();

  double        inner_r   = x_dim.rmin();
  double        dphi      = (2*M_PI/nsides);
  double        hphi      = dphi/2;
  double        support_thickness = 0.0;
  if(x_staves.hasChild("support")){
    support_thickness = getAttrOrDefault(x_staves.child(_U(support)), _U(thickness), 5.0 * cm);
  }
  double        mod_z     = layering.totalThickness() + support_thickness;
  double        outer_r   = inner_r + mod_z;
  double        totThick  = mod_z;
  double        offset    = x_det.attr<double>(_Unicode(offset));
  DetElement    sdet      (det_name,det_id);
  Volume        motherVol = description.pickMotherVolume(sdet);
  PolyhedraRegular hedra  (nsides,inner_r,inner_r+totThick+tolerance*2e0,x_dim.z());
  Volume        envelope  (det_name,hedra,air);
  PlacedVolume  env_phv   = motherVol.placeVolume(envelope,Transform3D(Translation3D(0,0,offset)*RotationZ(M_PI/nsides)));

  env_phv.addPhysVolID("system",det_id);
  sdet.setPlacement(env_phv);

  DetElement    stave_det("stave0",det_id);
  double dx = 0.0; //mod_z / std::sin(dphi); // dx per layer

  // Compute the top and bottom face measurements.
  double trd_x2 = (2 * std::tan(hphi) * outer_r - dx)/2 - tolerance;
  double trd_x1 = (2 * std::tan(hphi) * inner_r + dx)/2 - tolerance;
  double trd_y1 = x_dim.z()/2 - tolerance;
  double trd_y2 = trd_y1;
  double trd_z  = mod_z/2 - tolerance;

  // Create the trapezoid for the stave.
  Trapezoid trd(trd_x1, // Outer side, i.e. the "long" X side.
                trd_x2, // Inner side, i.e. the "short"  X side.
                trd_y1, // Corresponds to subdetector (or module) Z.
                trd_y2, //
                trd_z); // Thickness, in Y for top stave, when rotated.

  Volume mod_vol("stave",trd,air);
  double l_pos_z = -(layering.totalThickness() / 2) - support_thickness/2.0;

  //double trd_x2_support = trd_x1;
  double trd_x1_support = (2 * std::tan(hphi) * outer_r - dx- support_thickness)/2 - tolerance;

  Solid  support_frame_s;
  // optional stave support
  if(x_staves.hasChild("support")){
    xml_comp_t x_support         = x_staves.child(_U(support));
    // is the support on the inside surface?
    bool       is_inside_support = getAttrOrDefault<bool>(x_support, _Unicode(inside), true);
    // number of "beams" running the length of the stave.
    int    n_beams        = getAttrOrDefault<int>(x_support, _Unicode(n_beams), 3);
    double beam_thickness = support_thickness / 4.0; // maybe a parameter later...
    trd_x1_support        = (2 * std::tan(hphi) * (outer_r - support_thickness + beam_thickness)) / 2 - tolerance;
    double grid_size      = getAttrOrDefault(x_support, _Unicode(grid_size), 25.0 * cm);
    double beam_width     = 2.0 * trd_x1_support / (n_beams + 1); // quick hack to make some gap between T beams

    double cross_beam_thickness    = support_thickness/4.0;
    //double trd_x1_support    = (2 * std::tan(hphi) * (inner_r + beam_thickness)) / 2 - tolerance;
    double trd_x2_support = trd_x2;

    int n_cross_supports = std::floor((trd_y1-cross_beam_thickness)/grid_size);

    Box        beam_vert_s(beam_thickness / 2.0 - tolerance, trd_y1, support_thickness / 2.0 - tolerance);
    Box        beam_hori_s(beam_width / 2.0 - tolerance, trd_y1, beam_thickness / 2.0 - tolerance);
    UnionSolid T_beam_s(beam_vert_s, beam_hori_s, Position(0, 0, -support_thickness / 2.0 + beam_thickness / 2.0));

    // cross supports
    Trapezoid  trd_support(trd_x1_support,trd_x2_support,
                           beam_thickness / 2.0 - tolerance, beam_thickness / 2.0 - tolerance,
                          support_thickness / 2.0 - tolerance - cross_beam_thickness/2.0);
    UnionSolid support_array_start_s(T_beam_s,trd_support,Position(0,0,cross_beam_thickness/2.0));
    for (int isup = 0; isup < n_cross_supports; isup++) {
      support_array_start_s = UnionSolid(support_array_start_s, trd_support, Position(0, -1.0 * isup * grid_size, cross_beam_thickness/2.0));
      support_array_start_s = UnionSolid(support_array_start_s, trd_support, Position(0, 1.0 * isup * grid_size, cross_beam_thickness/2.0));
    }
    support_array_start_s =
        UnionSolid(support_array_start_s, beam_hori_s,
                   Position(-1.8 * 0.5*(trd_x1+trd_x2_support) / n_beams, 0, -support_thickness / 2.0 + beam_thickness / 2.0));
    support_array_start_s =
        UnionSolid(support_array_start_s, beam_hori_s,
                   Position(1.8 * 0.5*(trd_x1+trd_x2_support) / n_beams, 0, -support_thickness / 2.0 + beam_thickness / 2.0));
    support_array_start_s =
        UnionSolid(support_array_start_s, beam_vert_s, Position(-1.8 * 0.5*(trd_x1+trd_x2_support) / n_beams, 0, 0));
    support_array_start_s =
        UnionSolid(support_array_start_s, beam_vert_s, Position(1.8 * 0.5*(trd_x1+trd_x2_support) / n_beams, 0, 0));

    support_frame_s = support_array_start_s;

    Material support_mat = description.material(x_support.materialStr());
    Volume   support_vol("support_frame_v", support_frame_s, support_mat);
    support_vol.setVisAttributes(description,x_support.visStr());

    // figure out how to best place
    //auto pv = mod_vol.placeVolume(support_vol, Position(0.0, 0.0, l_pos_z + support_thickness / 2.0));
    auto pv = mod_vol.placeVolume(support_vol, Position(0.0, 0.0, -l_pos_z - support_thickness / 2.0));
  }
  //l_pos_z += support_thickness;

  sens.setType("calorimeter");
  { // =====  buildBarrelStave(description, sens, module_volume) =====
    // Parameters for computing the layer X dimension:
    double stave_z  = trd_y1;
    double tan_hphi = std::tan(hphi);
    double l_dim_x  = trd_x1; // Starting X dimension for the layer.

    // Loop over the sets of layer elements in the detector.
    int l_num = 1;
    for(xml_coll_t li(x_det,_U(layer)); li; ++li)  {
      xml_comp_t x_layer = li;
      int repeat = x_layer.repeat();
      // Loop over number of repeats for this layer.
      for (int j=0; j<repeat; j++)    {
        string l_name = _toString(l_num,"layer%d");
        double l_thickness = layering.layer(l_num-1)->thickness();  // Layer's thickness.

        Position   l_pos(0,0,l_pos_z+l_thickness/2);      // Position of the layer.
	      double l_trd_x1 = l_dim_x - tolerance;
	      double l_trd_x2 = l_dim_x + l_thickness*tan_hphi - tolerance;
	      double l_trd_y1 = stave_z-tolerance;
	      double l_trd_y2 = l_trd_y1;
	      double l_trd_z  = l_thickness/2-tolerance;

        Trapezoid  l_trd(l_trd_x1,l_trd_x2,l_trd_y1,l_trd_y2,l_trd_z);
        Volume     l_vol(l_name,l_trd,air);
        DetElement layer(stave_det, l_name, det_id);

        // Loop over the sublayers or slices for this layer.
        int s_num = 1;
        double s_pos_z = -(l_thickness / 2);
        for(xml_coll_t si(x_layer,_U(slice)); si; ++si)  {
          xml_comp_t x_slice = si;
          string     s_name  = _toString(s_num,"slice%d");
          double     s_thick = x_slice.thickness();
          Volume     s_vol(s_name);
          DetElement slice(layer,s_name,det_id);

	          double s_trd_x1 = l_dim_x + (s_pos_z+l_thickness/2)*tan_hphi - tolerance;
	          double s_trd_x2 = l_dim_x + (s_pos_z+l_thickness/2+s_thick)*tan_hphi - tolerance;
	          double s_trd_y1 = stave_z-tolerance;
	          double s_trd_y2 = s_trd_y1;
	          double s_trd_z  = s_thick/2-tolerance;


            Trapezoid  s_trd(s_trd_x1, s_trd_x2, s_trd_y1, s_trd_y2, s_trd_z);
	          s_vol.setSolid(s_trd);
	          s_vol.setMaterial(description.material(x_slice.materialStr()));


          if (x_slice.hasChild("fiber")) {
            xml_comp_t x_fiber = x_slice.child(_Unicode(fiber));
            double f_radius = getAttrOrDefault(x_fiber, _U(radius), 0.1 * cm);
            double f_spacing_x = getAttrOrDefault(x_fiber, _Unicode(spacing_x), 0.122 * cm);
            double f_spacing_z = getAttrOrDefault(x_fiber, _Unicode(spacing_z), 0.134 * cm);
            std::string f_id_grid = getAttrOrDefault(x_fiber, _Unicode(identifier_grid), "grid");
            std::string f_id_fiber = getAttrOrDefault(x_fiber, _Unicode(identifier_fiber), "fiber");

            // Calculate fiber positions inside the slice
            Tube f_tube(0, f_radius, stave_z-tolerance);

            // Set up the readout grid for the fiber layers
            // Trapezoid is divided into segments with equal dz and equal number of divisions in x
            // Every segment is a polygon that can be attached later to the lightguide
            // The grid size is assumed to be ~2x2 cm (starting values). This is to be larger than
            // SiPM chip (for GlueX 13mmx13mm: 4x4 grid 3mmx3mm with 3600 50×50 μm pixels each)
            // See, e.g., https://arxiv.org/abs/1801.03088 Fig. 2d

            // Calculate number of divisions
            pair<int, int> grid_div = getNdivisions(s_trd_x1, s_thick-tolerance, 2.0*cm, 2.0*cm);
            // Calculate polygonal grid coordinates (vertices)
            vector<tuple<int, Point, Point, Point, Point>> grid_vtx = gridPoints(grid_div.first, grid_div.second, s_trd_x1, s_thick-tolerance, hphi);

            vector<int> f_id_count(grid_div.first*grid_div.second,0);
            auto f_pos = fiberPositions(f_radius, f_spacing_x, f_spacing_z, s_trd_x1, s_thick-tolerance, hphi);
            for (auto &line : f_pos) {
              for (auto &p : line) {
                int f_grid_id = -1;
                int f_id = -1;
                // Check to which grid fiber belongs to
                for (auto &poly_vtx : grid_vtx) {
                  auto [grid_id, vtx_a, vtx_b, vtx_c, vtx_d] = poly_vtx;
                  double poly_x[4] = {vtx_a.x(), vtx_b.x(), vtx_c.x(), vtx_d.x()};
                  double poly_y[4] = {vtx_a.y(), vtx_b.y(), vtx_c.y(), vtx_d.y()};
                  double f_xy[2] = {p.x(), p.y()};

                  TGeoPolygon poly(4);
                  poly.SetXY(poly_x,poly_y);
                  poly.FinishPolygon();

                  if(poly.Contains(f_xy)) {
                    f_grid_id = grid_id;
                    f_id = f_id_count[grid_id];
                    f_id_count[grid_id]++;
                  }
                }

                string f_name = "fiber" + to_string(f_grid_id) + "_" + to_string(f_id);
                Volume f_vol(f_name, f_tube, description.material(x_fiber.materialStr()));
                DetElement fiber(slice, f_name, det_id);
                if ( x_fiber.isSensitive() ) {
                  f_vol.setSensitiveDetector(sens);
                }
                fiber.setAttributes(description,f_vol,x_fiber.regionStr(),x_fiber.limitsStr(),x_fiber.visStr());

                // Fiber placement
                Transform3D f_tr(RotationZYX(0,0,M_PI*0.5),Position(p.x(), 0 ,p.y()));
                PlacedVolume fiber_phv = s_vol.placeVolume(f_vol, f_tr);
                fiber_phv.addPhysVolID(f_id_grid, f_grid_id + 1).addPhysVolID(f_id_fiber, f_id + 1);
                fiber.setPlacement(fiber_phv);
              }
	        }
          }

          if ( x_slice.isSensitive() ) {
            s_vol.setSensitiveDetector(sens);
          }

          slice.setAttributes(description,s_vol,x_slice.regionStr(),x_slice.limitsStr(),x_slice.visStr());

          // Slice placement.
          PlacedVolume slice_phv = l_vol.placeVolume(s_vol,Position(0,0,s_pos_z+s_thick/2));
          slice_phv.addPhysVolID("slice", s_num);
          slice.setPlacement(slice_phv);
          // Increment Z position of slice.
          s_pos_z += s_thick;

          // Increment slice number.
          ++s_num;
        }

        // Set region, limitset, and vis of layer.
        layer.setAttributes(description,l_vol,x_layer.regionStr(),x_layer.limitsStr(),x_layer.visStr());

        PlacedVolume layer_phv = mod_vol.placeVolume(l_vol,l_pos);
        layer_phv.addPhysVolID("layer", l_num);
        layer.setPlacement(layer_phv);
        // Increment to next layer Z position.
        double xcut = l_thickness * tan_hphi;
        l_dim_x += xcut;
        l_pos_z += l_thickness;
        ++l_num;
      }
    }
  }

  // Set stave visualization.
  if ( x_staves )   {
    mod_vol.setVisAttributes(description.visAttributes(x_staves.visStr()));
  }
  // Phi start for a stave.
  double phi = M_PI / nsides;
  double mod_x_off = dx / 2;             // Stave X offset, derived from the dx.
  double mod_y_off = inner_r + mod_z/2;  // Stave Y offset

  // Create nsides staves.
  for (int i = 0; i < nsides; i++, phi -= dphi)      { // i is module number
    // Compute the stave position
    double m_pos_x = mod_x_off * std::cos(phi) - mod_y_off * std::sin(phi);
    double m_pos_y = mod_x_off * std::sin(phi) + mod_y_off * std::cos(phi);
    Transform3D tr(RotationZYX(0,phi,M_PI*0.5),Translation3D(-m_pos_x,-m_pos_y,0));
    PlacedVolume pv = envelope.placeVolume(mod_vol,tr);
    pv.addPhysVolID("system",det_id);
    pv.addPhysVolID("module",i+1);
    DetElement sd = i==0 ? stave_det : stave_det.clone(_toString(i,"stave%d"));
    sd.setPlacement(pv);
    sdet.add(sd);
  }

  // Set envelope volume attributes.
  envelope.setAttributes(description,x_det.regionStr(),x_det.limitsStr(),x_det.visStr());
  return sdet;
}

DECLARE_DETELEMENT(athena_EcalBarrelHybrid,create_detector)
