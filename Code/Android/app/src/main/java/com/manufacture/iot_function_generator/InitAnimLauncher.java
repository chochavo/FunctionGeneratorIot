package com.manufacture.iot_function_generator;

import android.content.Context;
import android.content.Intent;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.support.v7.app.AppCompatActivity;
import android.view.WindowManager;
import android.view.animation.TranslateAnimation;
import android.widget.ImageView;
import com.parameters.Constants;


public class InitAnimLauncher extends AppCompatActivity implements Constants {
    WifiManager wifiManager;
    TranslateAnimation animation;
    ImageView gifImageView;
    CountDownTimer countDownTimer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
        setContentView(R.layout.greetings_activity);
        wifiManager = (WifiManager) getApplicationContext().getSystemService(Context.WIFI_SERVICE);
        assert wifiManager != null;
        if (!wifiManager.isWifiEnabled()) wifiManager.setWifiEnabled(true);
        animation = new TranslateAnimation(0f, 0f, 600f, 0);
        animation.setDuration(750);
        animation.setRepeatCount(0);
        animation.setRepeatMode(0);
        animation.setFillAfter(true);
        gifImageView = findViewById(R.id.imageView);
        gifImageView.startAnimation(animation);
        countDownTimer = new CountDownTimer(LAUNCH_SCREEN_ANIM_DURATION, 500 )
        {
            public void onTick(long millisUntilFinished) {}
            public void onFinish() {
                cancel();
                jumpToMainMenu();
            }
        }.start();
    }
public void jumpToMainMenu()
{
    Intent mIntent = new Intent(this, MainMenu.class);
    startActivity(mIntent);
    finish();
}
}

