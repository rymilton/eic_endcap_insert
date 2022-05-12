'''
    A script to calcualte placement of ecal endcap modules
    lxml is not included in container, get it by simply typing 'pip install lxml'
    Author: Chao Peng (ANL)
    Date: 06/17/2021
'''

import os
import numpy as np
import argparse
import DDG4
from lxml import etree as ET
from matplotlib import pyplot as plt
from matplotlib.collections import PatchCollection
from matplotlib.patches import Rectangle, Circle


CRYSTAL_SIZE = (20., 20., 200.) # mm
CRYSTAL_GAP = 0.5 # mm
CRYSTAL_ALIGNMENT = [
    (5, 21), (5, 21), (5, 21), (4, 22),
    (3, 23), (0, 26), (0, 24), (0, 24),
    (0, 24), (0, 24), (0, 24), (0, 24),
    (0, 22), (0, 22), (0, 20), (0, 20),
    (0, 18), (0, 18), (0, 16), (0, 16),
    (0, 14), (0, 14), (0, 12), (0, 12),
    (0,  6), (0,  6),
]

GLASS_SIZE = (40., 40., 400.) # mm
GLASS_GAP = 1.0 # mm
GLASS_ALIGNMENT = [
    (13, 10), (13, 10), (13, 10), (12, 10),
    (12, 10), (12, 10), (11, 11), (10, 11),
    (9, 12),  (8, 13),  (7, 13),  (6, 14),
    (3, 16),  (0, 18),  (0, 18),  (0, 16),
    (0, 16),  (0, 14),  (0, 13),  (0, 11),
    (0, 10),  (0, 7),   (0, 3),
]

# calculate positions of modules with a quad-alignment and module size
def individual_placement(alignment, module_x, module_y, gap=0.):
    placements = []
    for row, (start, num) in enumerate(alignment):
        for col in np.arange(start, start + num):
            placements.append(((col + 0.5)*(module_y + gap), (row + 0.5)*(module_x + gap)))
    placements = np.asarray(placements)
    return np.vstack((placements,
            np.vstack((placements.T[0]*-1., placements.T[1])).T,
            np.vstack((placements.T[0], placements.T[1]*-1.)).T,
            np.vstack((placements.T[0]*-1., placements.T[1]*-1.)).T))


def draw_placement(axis, colors=('teal'), module_alignment=((CRYSTAL_SIZE, CRYSTAL_GAP, CRYSTAL_ALIGNMENT))):
    xmin, ymin, xmax, ymax = 0., 0., 0., 0.
    patches = []
    numbers = []
    for color, (mod_size, mod_gap, alignment) in zip(colors, module_alignment):
        placements = individual_placement(alignment, *mod_size[:2], mod_gap)
        boxes = [Rectangle((x - (mod_size[0] + mod_gap)/2., y - (mod_size[1] + mod_gap)/2.), mod_size[0], mod_size[1])
                 for x, y in placements]
        patches.append(Rectangle((0., 0.), *mod_size[:2], facecolor=color, alpha=0.5, edgecolor='k'))
        numbers.append(len(placements))
        pc = PatchCollection(boxes, facecolor=color, alpha=0.5, edgecolor='k')

        xmin = min(xmin, placements.T[0].min() - 8.*(mod_size[0] + mod_gap))
        ymin = min(ymin, placements.T[1].min() - 8.*(mod_size[1] + mod_gap))
        xmax = max(xmax, placements.T[0].max() + 8.*(mod_size[0] + mod_gap))
        ymax = max(ymax, placements.T[1].max() + 8.*(mod_size[1] + mod_gap))

        # Add collection to axes
        axis.add_collection(pc)
    axis.set_xlim(xmin, xmax)
    axis.set_ylim(ymin, ymax)
    return axis, patches, numbers


def compact_constants(path, names):
    if not os.path.exists(path):
        print('Cannot find compact file \"{}\".'.format(path))
        return []
    kernel = DDG4.Kernel()
    description = kernel.detectorDescription()
    kernel.loadGeometry("file:{}".format(path))
    try:
        vals = [description.constantAsDouble(n) for n in names]
    except:
        print('Fail to extract values from {}, return empty.'.format(names))
        vals = []
    kernel.terminate()
    return vals


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-s', '--save', default='ce_ecal_placement_test.xml',
            help='path to save compact file.')
    parser.add_argument('-c', '--compact', default='',
            help='compact file to get contant to plot')
    parser.add_argument('--radii-constants', dest='radii', default='EcalBarrel_rmin',
            help='constant names in compact file to plot, seprate by \",\"')
    parser.add_argument('--individual', dest='indiv', action='store_true',
            help='individual block placements instead of line placements')
    args = parser.parse_args()

    data = ET.Element('lccdd')
    defines = ET.SubElement(data, 'define')

    # constants: name, value
    CONSTANTS = [
        ('CrystalModule_sx', '{:.2f}*mm'.format(CRYSTAL_SIZE[0])),
        ('CrystalModule_sy', '{:.2f}*mm'.format(CRYSTAL_SIZE[1])),
        ('CrystalModule_sz', '{:.2f}*mm'.format(CRYSTAL_SIZE[2])),
        ('CrystalModule_wrap', '{:.2f}*mm'.format(CRYSTAL_GAP)),

        ('GlassModule_sx', '{:.2f}*mm'.format(GLASS_SIZE[0])),
        ('GlassModule_sy', '{:.2f}*mm'.format(GLASS_SIZE[1])),
        ('GlassModule_sz', '{:.2f}*mm'.format(GLASS_SIZE[2])),
        ('GlassModule_wrap', '{:.2f}*mm'.format(GLASS_GAP)),

        ('CrystalModule_z0', '10.*cm'),
        ('GlassModule_z0', '0.0*cm'),
        ('EcalEndcapN_z0', '-EcalEndcapN_zmin-max(CrystalModule_sz,GlassModule_sz)/2.'),

        ('CrystalModule_dx', 'CrystalModule_sx + CrystalModule_wrap'),
        ('CrystalModule_dy', 'CrystalModule_sy + CrystalModule_wrap'),
        ('GlassModule_dx', 'GlassModule_sx + GlassModule_wrap'),
        ('GlassModule_dy', 'GlassModule_sy + GlassModule_wrap'),
    ]

# line-by-line alignment start pos, total number of blocks
    for name, value in CONSTANTS:
        constant = ET.SubElement(defines, 'constant')
        constant.set('name', name)
        constant.set('value', value)

    # this value will be used multiple times, so define it here
    readout_name = 'EcalEndcapNHits'

    # detector and its dimension/position/rotation
    dets = ET.SubElement(data, 'detectors')
    cmt = ET.SubElement(dets, 'comment')
    cmt.text = ' Backwards Endcap EM Calorimeter, placements generated by script '

    det = ET.SubElement(dets, 'detector')
    det.set('id', 'ECalEndcapN_ID')
    det.set('name', 'EcalEndcapN')
    det.set('type', 'HomogeneousCalorimeter')
    det.set('readout', readout_name)

    pos = ET.SubElement(det, 'position')
    pos.set('x', '0')
    pos.set('y', '0')
    pos.set('z', 'EcalEndcapN_z0')

    rot = ET.SubElement(det, 'rotation')
    rot.set('x', '0')
    rot.set('y', '0')
    rot.set('z', '0')

    # placements of modules
    plm = ET.SubElement(det, 'placements')
    pltype = 'individuals' if args.indiv else 'lines'

    # crystal
    crystal = ET.SubElement(plm, pltype)
    crystal.set('sector', '1')
    crystal_mod = ET.SubElement(crystal, 'module')
    crystal_mod.set('sizex', 'CrystalModule_sx')
    crystal_mod.set('sizey', 'CrystalModule_sy')
    crystal_mod.set('sizez', 'CrystalModule_sz')
    crystal_mod.set('material', 'PbWO4')
    crystal_mod.set('vis', 'AnlTeal')
    crystal_wrap = ET.SubElement(crystal, 'wrapper')
    crystal_wrap.set('thickness', 'CrystalModule_wrap')
    crystal_wrap.set('material', 'Epoxy')
    crystal_wrap.set('vis', 'WhiteVis')
    # crystal placements (for individuals)
    if args.indiv:
        for m, (x, y) in enumerate(individual_placement(CRYSTAL_ALIGNMENT, *CRYSTAL_SIZE[:2], CRYSTAL_GAP)):
            module = ET.SubElement(crystal, 'placement')
            module.set('x', '{:.3f}*mm'.format(x))
            module.set('y', '{:.3f}*mm'.format(y))
            module.set('z', 'CrystalModule_z0')
            module.set('id', '{:d}'.format(m))
    # crystal placements (for lines)
    else:
        crystal.set('mirrorx', 'true')
        crystal.set('mirrory', 'true')
        for row, (begin, nmods) in enumerate(CRYSTAL_ALIGNMENT):
            line = ET.SubElement(crystal, 'line')
            line.set('axis', 'x')
            line.set('x', 'CrystalModule_dx/2.')
            line.set('y', 'CrystalModule_dy*{:d}/2.'.format(row*2 + 1))
            line.set('z', 'CrystalModule_z0')
            line.set('begin', '{:d}'.format(begin))
            line.set('nmods', '{:d}'.format(nmods))


    # glass
    glass = ET.SubElement(plm, pltype)
    glass.set('sector', '2')
    glass_mod = ET.SubElement(glass, 'module')
    glass_mod.set('sizex', 'GlassModule_sx')
    glass_mod.set('sizey', 'GlassModule_sy')
    glass_mod.set('sizez', 'GlassModule_sz')
    # TODO: change glass material
    glass_mod.set('material', 'PbGlass')
    glass_mod.set('vis', 'AnlBlue')
    glass_wrap = ET.SubElement(glass, 'wrapper')
    glass_wrap.set('thickness', 'GlassModule_wrap')
    glass_wrap.set('material', 'Epoxy')
    glass_wrap.set('vis', 'WhiteVis')
    # crystal placements (for individuals)
    if args.indiv:
        for m, (x, y) in enumerate(individual_placement(GLASS_ALIGNMENT, *GLASS_SIZE[:2], GLASS_GAP)):
            module = ET.SubElement(glass, 'placement')
            module.set('x', '{:.3f}*mm'.format(x))
            module.set('y', '{:.3f}*mm'.format(y))
            module.set('z', 'GlassModule_z0')
            module.set('id', '{:d}'.format(m))
    # crystal placements (for lines)
    else:
        glass.set('mirrorx', 'true')
        glass.set('mirrory', 'true')
        for row, (begin, nmods) in enumerate(GLASS_ALIGNMENT):
            line = ET.SubElement(glass, 'line')
            line.set('axis', 'x')
            line.set('x', 'GlassModule_dx/2.')
            line.set('y', 'GlassModule_dy*{:d}/2.'.format(row*2 + 1))
            line.set('z', 'GlassModule_z0')
            line.set('begin', '{:d}'.format(begin))
            line.set('nmods', '{:d}'.format(nmods))


    # readout
    readouts = ET.SubElement(data, 'readouts')
    cmt = ET.SubElement(readouts, 'comment')
    cmt.text = 'Effectively no segmentation, the segmentation is used to provide cell dimension info'
    readout = ET.SubElement(readouts, 'readout')
    readout.set('name', readout_name)
    seg = ET.SubElement(readout, 'segmentation')
    # need segmentation to provide cell dimension info
    # seg.set('type', 'NoSegmentation')
    seg.set('type', 'MultiSegmentation')
    seg.set('key', 'sector')
    crystal_seg = ET.SubElement(seg, 'segmentation')
    crystal_seg.set('name', 'CrystalSeg')
    crystal_seg.set('key_value', '1')
    crystal_seg.set('type', 'CartesianGridXY')
    crystal_seg.set('grid_size_x', 'CrystalModule_dx')
    crystal_seg.set('grid_size_y', 'CrystalModule_dy')
    glass_seg = ET.SubElement(seg, 'segmentation')
    glass_seg.set('name', 'GlassSeg')
    glass_seg.set('key_value', '2')
    glass_seg.set('type', 'CartesianGridXY')
    glass_seg.set('grid_size_x', 'GlassModule_dx')
    glass_seg.set('grid_size_y', 'GlassModule_dy')
    rid = ET.SubElement(readout, 'id')
    rid.text = 'system:8,sector:4,module:20,x:32:-16,y:-16'


    text = ET.tostring(data, pretty_print=True)
    with open(args.save, 'wb') as f:
        f.write(text)


    fig, ax = plt.subplots(figsize=(12, 12), dpi=160)
    ax, patches, nblocks = draw_placement(ax, ['teal', 'royalblue'],
            [(CRYSTAL_SIZE, CRYSTAL_GAP, CRYSTAL_ALIGNMENT), (GLASS_SIZE, GLASS_GAP, GLASS_ALIGNMENT)])
    ax.set_xlabel('x (mm)', fontsize=24)
    ax.set_ylabel('y (mm)', fontsize=24)
    ax.tick_params(direction='in', labelsize=22, which='both')
    ax.set_axisbelow(True)
    ax.grid(linestyle=':', which='both')
    ax.legend(patches, ['{} {}'.format(num, name) for num, name in zip(nblocks, ['PbWO$_4$', 'SciGlass'])], fontsize=24)

    if args.compact and args.radii:
        names = [c.strip() for c in args.radii.split(',')]
        radii = compact_constants(args.compact, names)
        for name, radius in zip(names, radii):
            ax.add_patch(Circle((0, 0), radius*10., facecolor='none', edgecolor='k', linewidth=2))
            ax.annotate(name, xy=(radius*10/1.4, radius*10/1.4), fontsize=22)
    fig.savefig('ce_ecal_placement.png')

