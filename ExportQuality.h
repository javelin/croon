/*
 * File  : ExportQuality.h
 * Author: Mark Documento
 */

#ifndef _Croon_ExportQuality_h_
#define _Croon_ExportQuality_h_

// Export target for a project video. The low-res variants render a 180p
// preview (optionally time-limited); FullSource keeps the source video's own
// resolution; the fixed tiers rescale to their nominal height.
//
// Adding a new resolution tier (e.g. Full8K) is a one-line change: add the
// enum value below and its height in ExportScaleHeight(). Nothing in the export
// pipeline takes a per-resolution flag, so no signatures change.
enum ExportQuality {
    LowRes30Sec,
    LowRes60Sec,
    LowResFull,
    FullSource,
    FullHD,
    Full4K
};

inline bool ExportIsLowRes(ExportQuality quality) {
    return quality == LowRes30Sec ||
           quality == LowRes60Sec ||
           quality == LowResFull;
}

// Seconds to cap a preview render at; 0 means render the full duration.
inline int ExportPreviewSeconds(ExportQuality quality) {
    if (quality == LowRes30Sec) return 30;
    if (quality == LowRes60Sec) return 60;
    return 0;
}

// Height, in pixels, to rescale the export to. Previews are 180p; the fixed
// tiers rescale to their nominal height; FullSource returns 0, meaning "keep
// the source resolution" (background exports only — not valid for a generated
// visualization). Extend the switch to add tiers: Full8K -> 4320, etc.
inline int ExportScaleHeight(ExportQuality quality) {
    switch (quality) {
        case LowRes30Sec:
        case LowRes60Sec:
        case LowResFull: return 180;
        case FullSource: return 0;
        case Full4K:     return 2160;
        // case Full8K:  return 4320;
        case FullHD:
        default:         return 1080;
    }
}

#endif
