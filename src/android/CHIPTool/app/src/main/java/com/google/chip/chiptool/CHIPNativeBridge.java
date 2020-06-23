package com.google.chip.chiptool;

public class CHIPNativeBridge {
    static {
        System.loadLibrary("native-lib");
    }

    private static CHIPNativeBridge instance = null;

    private CHIPNativeBridge() { }

    public static CHIPNativeBridge getInstance() {
        if (instance == null)
            instance = new CHIPNativeBridge();
        return instance;
    }

    // TODO: Sample function to demonstrate end-to-end calls work
    public native String base41Encode();
}
