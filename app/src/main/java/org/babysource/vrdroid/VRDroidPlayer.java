/**
 * Created on 2016/3/31
 */
package org.babysource.vrdroid;

import android.os.Bundle;

import org.babysource.vrdroid.render.VRDroidRender;
import org.rajawali3d.vr.VRActivity;

/**
 * VR资源播放
 * <p/>
 *
 * @author Wythe
 */
public class VRDroidPlayer extends VRActivity {

    @Override
    public void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setRenderer(new VRDroidRender(this));
    }

}
