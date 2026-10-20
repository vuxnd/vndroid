package com.vndroid.video;

import com.vndroid.control.PositionMapper;

public interface VirtualDisplayListener {
    void onNewVirtualDisplay(int displayId, PositionMapper positionMapper);
}
