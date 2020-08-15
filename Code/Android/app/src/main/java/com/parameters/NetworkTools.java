package com.parameters;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.util.Log;

import com.manufacture.iot_function_generator.MainMenu;

import java.io.IOException;
import java.net.HttpURLConnection;
import java.net.InetAddress;
import java.net.URL;
import java.net.UnknownHostException;
import java.util.List;
import static android.content.Context.WIFI_SERVICE;

public class NetworkTools implements Constants {


    public static boolean connectionType = TYPE_DIRECT;
    public static String customSSID;
    public static String customPASS;
    public static char customEncrypt;
    public static String s_gateway;
    public static String s_ipAddress;
    public static String deviceMAC;
    public static String serverIP;
    public static String macAddressString = "";
    public static String deviceIp = "";
    private Context context;

    public void connectToWifiNetwork(String ssid, String pass, char encrypt, Context contextIn) {
        WifiConfiguration wifiConfig = new WifiConfiguration();
        switch(encrypt) {
            case WPA: case WPA_2: wifiConfig.preSharedKey = String.format("\"%s\"",  pass); break;
            case WEP:
                wifiConfig.wepKeys[0] = "\"" + pass + "\"";
                wifiConfig.wepTxKeyIndex = 0;
                wifiConfig.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.NONE);
                wifiConfig.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.WEP40);
                break;
            case OPEN: wifiConfig.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.NONE); break;
        }
        context = contextIn;
        WifiManager wifiManager = (WifiManager) context.getApplicationContext().getSystemService(WIFI_SERVICE); // Modified line.
        assert wifiManager != null;
        wifiManager.addNetwork(wifiConfig);
        List<WifiConfiguration> list = wifiManager.getConfiguredNetworks();
        for( WifiConfiguration i : list ) {
            if(i.SSID != null && i.SSID.equals("\"" +  ssid + "\"")) {
                wifiManager.disconnect();
                wifiManager.enableNetwork(i.networkId, true);
                wifiManager.reconnect();
                wifiManager.saveConfiguration();
                break;
            }
        }
    }


    private boolean checkWifiOnAndConnected() {
        WifiManager wifiManager = (WifiManager) context.getApplicationContext().getSystemService(WIFI_SERVICE);
        assert wifiManager != null;
        if (wifiManager.isWifiEnabled()) { // Wi-Fi simpleAdapter is ON
            WifiInfo wifiInfo = wifiManager.getConnectionInfo();
            return wifiInfo.getNetworkId() != -1;
        }
        else return false; // Wi-Fi simpleAdapter is OFF
    }

    public boolean isNetworkAvailable(Context contextIn) {
        this.context = contextIn;
        Log.v(TAG, "Check network state");
        if (checkWifiOnAndConnected()) {
            if (isWifi2G())
            {
                if (isOnline()) {
                    Log.v(TAG, "Internet & WiFi are available");
                    return true;
                }
                else return false;
            }
            else return false;
        }
        else {
            Log.w(TAG, "Internet & WiFi are Not available!");
            return false;
        }
    }

    private boolean isNetworkOK() {
        ConnectivityManager connectivityManager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        assert connectivityManager != null;
        NetworkInfo activeNetworkInfo = connectivityManager.getActiveNetworkInfo();
        return activeNetworkInfo != null;
    }

    //Context context = this;
    private boolean isOnline()
    {
        if (ifDeviceSdkAboveMarshmallow()) {
            if (isNetworkOK()) {
                try {
                    HttpURLConnection urlc = (HttpURLConnection) (new URL("http://www.google.com").openConnection());
                    urlc.setRequestProperty("User-Agent", "Test");
                    urlc.setRequestProperty("Connection", "close");
                    urlc.setConnectTimeout(1500);
                    urlc.connect();
                    return (urlc.getResponseCode() == 200);
                } catch (IOException e) {
                    Log.e(TAG, "Error checking internet connection", e);
                    return false;
                }
            } else Log.d(TAG, "No network available!");
        }
        else return isDnsFromNameAvailable();
        return false;
    }

    private static boolean ifDeviceSdkAboveMarshmallow() {
        String release = Build.VERSION.RELEASE;
        int sdkVersion = Build.VERSION.SDK_INT;
        Log.i(TAG, "Android SDK: " + sdkVersion + " (" + release +")");
        return Build.VERSION.SDK_INT > Build.VERSION_CODES.M;
    }

    private boolean isDnsFromNameAvailable() {
        final ConnectivityManager connectivityManager = ((ConnectivityManager) context.getApplicationContext().getSystemService(Context.CONNECTIVITY_SERVICE));
        assert connectivityManager != null;
        try {
            final InetAddress address = InetAddress.getByName("www.google.com");
            return (!address.toString().equals("")) && connectivityManager.getActiveNetworkInfo() != null && connectivityManager.getActiveNetworkInfo().isConnected();
        } catch (UnknownHostException e) {
            Log.e(TAG, String.valueOf(e));
        }
        return false;
    }

    private boolean isWifi2G() {
        WifiManager wifiManager = (WifiManager) context.getApplicationContext().getSystemService(WIFI_SERVICE);
        assert wifiManager != null;
        WifiInfo wifiInfo = wifiManager.getConnectionInfo();
        int Freq = wifiInfo.getFrequency();
        Log.i(TAG,"Current network frequency is: " + Freq);
        return (Freq >= FREQUENCY_LIMIT_MIN) && (Freq <= FREQUENCY_LIMIT_MAX);
    }

}
