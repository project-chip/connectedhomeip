package com.tcl.chip.chiptvserver.service;

import android.content.Context;

import androidx.annotation.NonNull;

import com.tcl.chip.tvapp.KeypadInputManagerStub;
import com.tcl.chip.tvapp.MediaInputManagerStub;
import com.tcl.chip.tvapp.TvApp;
import com.tcl.chip.tvapp.WakeOnLanManagerStub;

import chip.appserver.ChipAppServer;
import chip.platform.AndroidBleManager;
import chip.platform.AndroidChipPlatform;
import chip.platform.ChipMdnsCallbackImpl;
import chip.platform.NsdManagerServiceResolver;
import chip.platform.PreferencesConfigurationManager;
import chip.platform.PreferencesKeyValueStoreManager;

public class MatterServant {

    private MatterServant() {
    }

    private static class SingletonHolder{
        static MatterServant instance = new MatterServant();
    }

    public static MatterServant get(){
        return SingletonHolder.instance;
    }

    public void init(@NonNull Context context){
        TvApp tvApp = new TvApp();
        tvApp.setKeypadInputManager(new KeypadInputManagerStub());
        tvApp.setMediaInputManager(new MediaInputManagerStub());
        tvApp.setWakeOnLanManager(new WakeOnLanManagerStub());

        Context applicationContext = context.getApplicationContext();
        AndroidChipPlatform chipPlatform =
                new AndroidChipPlatform(
                        new AndroidBleManager(),
                        new PreferencesKeyValueStoreManager(applicationContext),
                        new PreferencesConfigurationManager(applicationContext),
                        new NsdManagerServiceResolver(applicationContext),
                        new ChipMdnsCallbackImpl());

        ChipAppServer chipAppServer = new ChipAppServer();
        chipAppServer.startApp();
    }
}
