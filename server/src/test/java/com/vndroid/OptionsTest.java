package com.vndroid;

import org.junit.Assert;
import org.junit.Test;

public class OptionsTest {

    @Test
    public void testSaturationDefault() {
        Assert.assertEquals(1f, Options.parse(BuildConfig.VERSION_NAME).getSaturation(), 0);
    }

    @Test
    public void testSaturationAccepted() {
        Assert.assertEquals(0f, Options.parse(BuildConfig.VERSION_NAME, "saturation=0").getSaturation(), 0);
        Assert.assertEquals(1f, Options.parse(BuildConfig.VERSION_NAME, "saturation=1").getSaturation(), 0);
        Assert.assertEquals(2.5f, Options.parse(BuildConfig.VERSION_NAME, "saturation=2.5").getSaturation(), 0);
    }

    @Test
    public void testSaturationRejected() {
        Assert.assertThrows(IllegalArgumentException.class, () -> Options.parse(BuildConfig.VERSION_NAME, "saturation=-1"));
        Assert.assertThrows(IllegalArgumentException.class, () -> Options.parse(BuildConfig.VERSION_NAME, "saturation=NaN"));
        Assert.assertThrows(IllegalArgumentException.class, () -> Options.parse(BuildConfig.VERSION_NAME, "saturation=Infinity"));
        Assert.assertThrows(IllegalArgumentException.class, () -> Options.parse(BuildConfig.VERSION_NAME, "saturation=-Infinity"));
        Assert.assertThrows(IllegalArgumentException.class, () -> Options.parse(BuildConfig.VERSION_NAME, "saturation=abc"));
    }
}
