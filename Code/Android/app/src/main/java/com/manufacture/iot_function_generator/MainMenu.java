//-----------------------------------------------------------------------------
// SimpleMed+ WiFi Support Module V1.0
//-----------------------------------------------------------------------------
// Copyright 2018 Vaica Medical, Inc.
// http://www.VaicaMedical.com
//
// Target:         Android API Devices
// Release 1.0
//

package com.manufacture.iot_function_generator;

/* Base Android defined libraries */
import android.Manifest;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.IntentSender;
import android.content.pm.PackageManager;
import android.location.LocationManager;
import android.net.wifi.ScanResult;
import android.net.wifi.SupplicantState;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.text.Html;
import android.util.Log;
import android.view.ContextThemeWrapper;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

/* Google API defined libraries */
import com.google.android.gms.common.api.GoogleApiClient;
import com.google.android.gms.common.api.PendingResult;
import com.google.android.gms.common.api.Status;
import com.google.android.gms.location.LocationRequest;
import com.google.android.gms.location.LocationServices;
import com.google.android.gms.location.LocationSettingsRequest;
import com.google.android.gms.location.LocationSettingsResult;
import com.google.android.gms.location.LocationSettingsStatusCodes;
import com.parameters.Constants;
import com.parameters.NetworkTools;

/* JAVA defined libraries */
import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.List;
import java.util.Objects;

/* Main Activity provides user interface access to desired SM2 sequence */
public class MainMenu extends AppCompatActivity implements Constants {
    WifiScanReceiver wifiReceiver;              // WiFi Class
    WifiManager wifiManager;                           // WiFi class
    LayoutInflater layoutInflater;
    AlertDialog alertDialog;
    WifiInfo wifiInfo;
    WifiConfiguration wifiConfiguration;
    private GoogleApiClient googleApiClient;    // In-app location accessibility
    ProgressDialog progressDialog;
    List<WifiConfiguration> list;               // Lisr used in testing WiFi connection capabilities
    NetworkTools networkTools = new NetworkTools();
    // Code specified variables //
    public boolean isNetEncrypted = false;
    public boolean deriveConnectionFail = false;
    public boolean scanCompleted = false;

    /* User interface buttons */
    Button directConnection;
    Button exitApplication;
    Button wlanConnection;
    Button wlanPairing;

    private class WifiScanReceiver extends BroadcastReceiver {
        public void onReceive(Context c, Intent intent) {
            scanCompleted = true;
        }
    }

    protected void onPause() {
        super.onPause();
    }

    protected void onResume() {
        registerReceiver(wifiReceiver, new IntentFilter(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION));
        super.onResume();
    }

    /* TODO Main code block */
    @SuppressLint({"ResourceType", "ClickableViewAccessibility"})
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
        setContentView(R.layout.activity_main);

        /* Buttons behavior description */
        exitApplication = findViewById(R.id.button_exit);
        exitApplication.setOnTouchListener((v1, event) -> {
            switch(event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    exitApplication.setBackgroundResource(R.color.colorx);
                    return true; // if you want to handle the touch event
                case MotionEvent.ACTION_UP:
                    exitApplication.setBackgroundResource(R.color.buttonColor);
                    finish();
                    System.exit(0);
                    return true; // if you want to handle the touch event
            }
            return false;
        });

        directConnection = findViewById(R.id.button_direct_connection);
        directConnection.setOnTouchListener((v1, event) -> {
            switch(event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    directConnection.setBackgroundResource(R.color.colorx);
                    return true; // if you want to handle the touch event
                case MotionEvent.ACTION_UP:
                    directConnection.setBackgroundResource(R.color.buttonColor);
                    NetworkTools.connectionType = TYPE_DIRECT;
                    new connectToDeviceTask().execute("");
                    return true; // if you want to handle the touch event
            }
            return false;
        });

        wlanConnection = findViewById(R.id.button_wifi_pairing);
        wlanConnection.setOnTouchListener((v1, event) -> {
            switch(event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    wlanConnection.setBackgroundResource(R.color.colorx);
                    return true; // if you want to handle the touch event
                case MotionEvent.ACTION_UP:
                    wlanConnection.setBackgroundResource(R.color.buttonColor);
                    NetworkTools.connectionType = TYPE_WLAN;
                    initWifiAlert();
                    return true; // if you want to handle the touch event
            }
            return false;
        });

        wlanPairing = findViewById(R.id.button_recovery);
        wlanPairing.setOnTouchListener((v1, event) -> {
            switch(event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    wlanPairing.setBackgroundResource(R.color.colorx);
                    return true; // if you want to handle the touch event
                case MotionEvent.ACTION_UP:
                    wlanPairing.setBackgroundResource(R.color.buttonColor);
                    NetworkTools.connectionType = TYPE_WLAN;
                    checkAndRequestPermissions(this);
                    return true; // if you want to handle the touch event
            }
            return false;
        });
        registerReceiver(wifiReceiver, new IntentFilter(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION));
        getGpsStatus();
    }

    public void initWifiAlert() {
        if (readCredentialsFromFile(false) && readCredentialsFromFile(true)) {
            alertDialog = new AlertDialog.Builder(MainMenu.this).create();
            alertDialog.setTitle("Wi-Fi Configuration");
            alertDialog.setCancelable(false);
            alertDialog.setMessage(Html.fromHtml("<font color='#A6CCFF'>Saved network: " + NetworkTools.customSSID + ", MAC Address: " + NetworkTools.deviceMAC + "\n</font>"));
            alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "CONNECT",
                    (dialog, which) -> {
                dialog.dismiss();
                new connectToWifiTask().execute("");
            });
            alertDialog.setButton(AlertDialog.BUTTON_NEGATIVE, "CANCEL", (dialog, which) -> dialog.dismiss());
            Objects.requireNonNull(alertDialog.getWindow()).getAttributes().windowAnimations = (R.style.DialogTheme);
            alertDialog.getWindow().setBackgroundDrawableResource(R.color.colorx);
            alertDialog.show();
        }
    }


    /*  GPS Dedicated functions */
    public void getPermissionStatus() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M && checkSelfPermission(Manifest.permission.ACCESS_COARSE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
            requestPermissions(new String[]{Manifest.permission.ACCESS_COARSE_LOCATION}, PERMISSIONS_REQUEST_CODE_ACCESS_COARSE_LOCATION);
        }
    }

    public void checkAndRequestPermissions(Activity activity) {
        System.out.println("PermissionsUtils checkAndRequestPermissions()");
        int permissionLocation = ContextCompat.checkSelfPermission(activity, Manifest.permission.ACCESS_COARSE_LOCATION);
        if (permissionLocation == PackageManager.PERMISSION_GRANTED) new ShowConfiguredWiFi().execute("");
        else getPermissionStatus();
    }


    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        if (requestCode == PERMISSIONS_REQUEST_CODE_ACCESS_COARSE_LOCATION) {
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED)
                new ShowConfiguredWiFi().execute("");
            else
                Toast.makeText(this, "Please, grant permissions to the location service...", Toast.LENGTH_SHORT).show();
        }
    }

    public void getGpsStatus() {
        final LocationManager manager = (LocationManager) MainMenu.this.getSystemService(Context.LOCATION_SERVICE);
        assert manager != null;
        if (manager.isProviderEnabled(LocationManager.GPS_PROVIDER) && hasGPSDevice(MainMenu.this)) Log.i(TAG, "Gps already enabled");
        if(!hasGPSDevice(MainMenu.this)) Log.i(TAG, "Gps is not supported");
        if (!manager.isProviderEnabled(LocationManager.GPS_PROVIDER) && hasGPSDevice(MainMenu.this)) {
            Log.i(TAG, "Gps already enabled");
            enableLoc();
        }
        else Log.i(TAG, "Gps already enabled");
    }

    private boolean hasGPSDevice(Context context) {
        final LocationManager mgr = (LocationManager) context
                .getSystemService(Context.LOCATION_SERVICE);
        if (mgr == null) return false;
        final List<String> providers = mgr.getAllProviders();
        return providers != null && providers.contains(LocationManager.GPS_PROVIDER);
    }

    private void enableLoc() {
        if (googleApiClient == null) {
            googleApiClient = new GoogleApiClient.Builder(MainMenu.this)
                    .addApi(LocationServices.API)
                    .addConnectionCallbacks(new GoogleApiClient.ConnectionCallbacks() {
                        @Override
                        public void onConnected(Bundle bundle) {}
                        @Override
                        public void onConnectionSuspended(int i) { googleApiClient.connect(); }})
                    .addOnConnectionFailedListener(connectionResult -> Log.d("Location error","Location error " + connectionResult.getErrorCode())).build();
            googleApiClient.connect();
            LocationRequest locationRequest = LocationRequest.create();
            locationRequest.setPriority(LocationRequest.PRIORITY_HIGH_ACCURACY);
            locationRequest.setInterval(30 * 1000);
            locationRequest.setFastestInterval(5 * 1000);
            LocationSettingsRequest.Builder builder = new LocationSettingsRequest.Builder().addLocationRequest(locationRequest);
            builder.setAlwaysShow(true);
            PendingResult<LocationSettingsResult> result = LocationServices.SettingsApi.checkLocationSettings(googleApiClient, builder.build());
            result.setResultCallback(result1 -> {
                final Status status = result1.getStatus();
                if (status.getStatusCode() == LocationSettingsStatusCodes.RESOLUTION_REQUIRED) {
                    try {
                        status.startResolutionForResult(MainMenu.this, REQUEST_LOCATION);
                    } catch (IntentSender.SendIntentException ignored) {
                    }
                }
            });
        }
    }

    @SuppressLint("StaticFieldLeak")
    private class connectToDeviceTask extends AsyncTask<String, Void, String> {
        @Override
        protected String doInBackground(String... params) {
            wifiManager = (WifiManager) getApplicationContext().getSystemService(Context.WIFI_SERVICE);
            wifiConfiguration = new WifiConfiguration();
            wifiConfiguration.SSID = "\"" + "IOT_FUNCGEN" + "\"";
            wifiConfiguration.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.NONE);
            wifiManager.addNetwork(wifiConfiguration);
            Log.d(TAG, "the device ssid retrieve is:" + wifiConfiguration.SSID);
            int NetCheckRetries = 0;
            while(isWifiConnectedDirect() && (NetCheckRetries < NUMBER_OF_NET_CHECK_RETRIES)) {
                Log.w(TAG,"Connecting to the device...");
                initiateDirectConnection();
                try {
                    Thread.sleep(NETWORK_CHECK_TIME_DIFFERENCE_MS);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                NetCheckRetries++;
                Log.w(TAG,"NetCheckRetries value: " + NetCheckRetries);
            }
            deriveConnectionFail = isWifiConnectedDirect();
            return "Executed";
        }

        @Override
        protected void onPostExecute(String result) {
            Log.i(TAG,"isNetEncrypted value is: " + isNetEncrypted);
            if (progressDialog.isShowing()) progressDialog.dismiss();
            if (deriveConnectionFail) {
                alertDialog = new AlertDialog.Builder(MainMenu.this).create();
                alertDialog.setTitle("Oops!");
                alertDialog.setMessage(Html.fromHtml("<font color='#A6CCFF'>Could not connect to Wi-Fi Access Point...</font>"));
                alertDialog.setButton(AlertDialog.BUTTON_NEUTRAL, "OK",
                        (dialog, which) -> { dialog.dismiss(); });
                Objects.requireNonNull(alertDialog.getWindow()).getAttributes().windowAnimations = (R.style.DialogTheme);
                alertDialog.getWindow().setBackgroundDrawableResource(R.color.colorx);
                alertDialog.show();
            }
            else initMainUiActivity();
        }

        @Override
        protected void onPreExecute() {
            showProgress("Connecting to the Function Generator Access Point...");
        }

        @Override
        protected void onProgressUpdate(Void... values) {}
    }

    @SuppressLint("StaticFieldLeak")
    private class connectToWifiTask extends AsyncTask<String, Void, String> {
        NetworkTools nt = new NetworkTools();
        @Override
        protected String doInBackground(String... params) {
            int NetCheckRetries = 0;
            while(isWifiConnected() && (NetCheckRetries < NUMBER_OF_NET_CHECK_RETRIES)) {
                Log.w(TAG,"Connecting to the Wi-Fi network...");
                nt.connectToWifiNetwork(NetworkTools.customSSID, NetworkTools.customPASS, NetworkTools.customEncrypt, getApplicationContext());
                try {
                    Thread.sleep(NETWORK_CHECK_TIME_DIFFERENCE_MS);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                NetCheckRetries++;
                Log.w(TAG,"NetCheckRetries value: " + NetCheckRetries);
            }
            deriveConnectionFail = isWifiConnected();
            return "Executed";
        }

        @Override
        protected void onPostExecute(String result) {
            Log.i(TAG,"isNetEncrypted value is: " + isNetEncrypted);
            if (progressDialog.isShowing()) progressDialog.dismiss();
            if (deriveConnectionFail) {
                alertDialog = new AlertDialog.Builder(MainMenu.this).create();
                alertDialog.setTitle("Oops!");
                alertDialog.setMessage(Html.fromHtml("<font color='#A6CCFF'>Could not connect to device Wi-Fi Access Point...</font>"));
                alertDialog.setButton(AlertDialog.BUTTON_NEUTRAL, "OK",
                        (dialog, which) -> { dialog.dismiss(); });
                Objects.requireNonNull(alertDialog.getWindow()).getAttributes().windowAnimations = (R.style.DialogTheme);
                alertDialog.getWindow().setBackgroundDrawableResource(R.color.colorx);
                alertDialog.show();
            }
            else initMainUiActivity();
        }

        @Override
        protected void onPreExecute() {
            showProgress("Connecting to the " + NetworkTools.customSSID + " network...");
        }

        @Override
        protected void onProgressUpdate(Void... values) {}
    }

    public void initMainUiActivity() {
        Intent mIntent = new Intent(this, FuncgenMainUI.class);
        finish();
        startActivity(mIntent);
    }

    void initiateDirectConnection() {
            list = wifiManager.getConfiguredNetworks();
            for (WifiConfiguration i : list) {
                if (i.SSID != null && i.SSID.equals(wifiConfiguration.SSID)) {
                    wifiManager.disconnect();
                    wifiManager.enableNetwork(i.networkId, true);
                    wifiManager.reconnect();
                    Log.v(TAG, "i.SSID value is: " + i.SSID);
                }
            }
            try {
                Thread.sleep(8000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
    }

    private boolean isWifiConnectedDirect() {
        wifiManager = (WifiManager) getApplicationContext().getSystemService(Context.WIFI_SERVICE);
        assert wifiManager != null;
        wifiInfo = wifiManager.getConnectionInfo();
        if (wifiInfo.getSupplicantState() == SupplicantState.COMPLETED) {
            String currentSSID = wifiInfo.getSSID();
            Log.i(TAG, " SSID: " + currentSSID);
            return !currentSSID.contains("IOT");
        }
        return true;
    }

    private boolean isWifiConnected() {
        wifiManager = (WifiManager) getApplicationContext().getSystemService(Context.WIFI_SERVICE);
        assert wifiManager != null;
        wifiInfo = wifiManager.getConnectionInfo();
        if (wifiInfo.getSupplicantState() == SupplicantState.COMPLETED) {
            String currentSSID = wifiInfo.getSSID();
            return currentSSID.equals(NetworkTools.customSSID);
        }
        return false;
    }
    /* WiFi sequence AsyncTask */
    @SuppressLint("StaticFieldLeak")
    private class ShowConfiguredWiFi extends AsyncTask<String, Void, String> {
        @Override
        protected String doInBackground(String... params) {
            int NetCheckRetries = 0;
            wifiManager = (WifiManager) getApplicationContext().getSystemService(Context.WIFI_SERVICE);
            wifiReceiver = new WifiScanReceiver();
            assert wifiManager != null;
            wifiInfo = wifiManager.getConnectionInfo();
            if (!wifiManager.isWifiEnabled()) wifiManager.setWifiEnabled(true);
            while(!(networkTools.isNetworkAvailable(getApplicationContext())) && (NetCheckRetries < NUMBER_OF_NET_CHECK_RETRIES)) {
                Log.w(TAG,"Checking internet connection...");
                wifiManager.startScan();
                try {
                    Thread.sleep(NETWORK_CHECK_TIME_DIFFERENCE_MS);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                NetCheckRetries++;
                Log.w(TAG,"NetCheckRetries value: " + NetCheckRetries);
                }
                if (!networkTools.isNetworkAvailable(getApplicationContext())) deriveConnectionFail = true;
                else if (NetCheckRetries < NUMBER_OF_NET_CHECK_RETRIES) {
                    NetworkTools.customSSID = wifiInfo.getSSID();
                    List<ScanResult> networkList = wifiManager.getScanResults();
                    if ( NetworkTools.customSSID.startsWith("\"") &&  NetworkTools.customSSID.endsWith("\"")) {
                        NetworkTools.customSSID =  NetworkTools.customSSID.substring(1,  NetworkTools.customSSID.length() - 1);
                    }
                    if (networkList != null) {
                        for (ScanResult network : networkList) {
                            Log.i(TAG, "Current SSID in loop is: " + network.SSID + " Compared to our: " +  NetworkTools.customSSID);
                            //check if current connected SSID
                            if ( NetworkTools.customSSID.equals(network.SSID)) {
                                //get capabilities of current connection
                                String capabilities = network.capabilities;
                                Log.d(TAG, network.SSID + " capabilities : " + capabilities);
                                if (capabilities.contains("WPA"))   NetworkTools.customEncrypt = '2';
                                else if (capabilities.contains("WEP"))   NetworkTools.customEncrypt = '3';
                                else  NetworkTools.customEncrypt = '4';
                                isNetEncrypted = NetworkTools.customEncrypt != '4';
                                Log.i(TAG, "Connected network: " +  NetworkTools.customSSID);
                            }
                        }
                    }
                }
                else deriveConnectionFail = true;
            return "Executed";
        }

        @Override
        protected void onPostExecute(String result) {
            Log.i(TAG,"isNetEncrypted value is: " + isNetEncrypted);
            if (progressDialog.isShowing()) progressDialog.dismiss();
            if (deriveConnectionFail) {
                alertDialog = new AlertDialog.Builder(MainMenu.this).create();
                alertDialog.setTitle("WiFi connection status");
                alertDialog.setMessage(Html.fromHtml("<font color='#A6CCFF'>No Internet available WiFi connection. Select desired network from the networksList...</font>"));
                alertDialog.setButton(AlertDialog.BUTTON_NEUTRAL, "OK",
                        (dialog, which) -> {
                            dialog.dismiss();
                            jumpToNextIntent(false);
                        });
                Objects.requireNonNull(alertDialog.getWindow()).getAttributes().windowAnimations = (R.style.DialogTheme);
                alertDialog.getWindow().setBackgroundDrawableResource(R.color.colorx);
                alertDialog.show();
            }
            else if (isNetEncrypted) getNetworkPasskey();
            else getOpenNetworkPermissions();
        }

        @Override
        protected void onPreExecute() {
            showProgress("Checking connected WiFi connection...");
        }

        @Override
        protected void onProgressUpdate(Void... values) {}
    }

    public void jumpToNextIntent(boolean ConFlag) {
        Intent mIntent = new Intent(this, WifiNetworksScanner.class);
        mIntent.putExtra("CONNECT_FLAG", ConFlag); // false - scan. true - provide SSID and PASS
        finish();
        startActivity(mIntent);
    }
    public void showProgress(String ProgMessage) {
        progressDialog = new ProgressDialog(new ContextThemeWrapper(this, android.R.style.Theme_Holo_Dialog));
        progressDialog.setMessage(ProgMessage);
        progressDialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
        progressDialog.setIndeterminate(true);
        progressDialog.setCancelable(false);
        progressDialog.show();
        final int totalProgressTime = 100;
        final Thread t = new Thread() {
            @Override
            public void run() {
                int jumpTime = 0;
                while (jumpTime < totalProgressTime) {
                    try {
                        sleep(200);
                        jumpTime += 1;
                        progressDialog.setProgress(jumpTime);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            }
        };
        t.start();
    }

    @SuppressLint("SetTextI18n")
    public void getOpenNetworkPermissions()
    {
        layoutInflater = LayoutInflater.from(this);
        @SuppressLint("InflateParams") View promptsView = layoutInflater.inflate(R.layout.prompts_no_pass, null);
        AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(MainMenu.this);
        alertDialogBuilder.setTitle("Connect to the WiFi network, by pressing " + '\"' + "OK" + '\"' + " button.");
        final TextView selectedSSID = promptsView.findViewById(R.id.selected_ssid);
        selectedSSID.setText("Selected network is: " +  NetworkTools.customSSID);
        alertDialogBuilder
                .setCancelable(false)
                .setPositiveButton("OK",
                        (dialog, id) -> {
                            Log.i(TAG, "ssid_result = " +  NetworkTools.customSSID);
                            NetworkTools.customEncrypt = '4'; // OPEN Net
                        })
                .setNeutralButton("Scan",
                        (dialog, id) -> jumpToNextIntent(false))
                .setNegativeButton("Cancel",
                        (dialog, id) -> dialog.cancel());
        alertDialog = alertDialogBuilder.create();
        Objects.requireNonNull(alertDialog.getWindow()).setBackgroundDrawableResource(R.color.colorx);
        alertDialog.getWindow().getAttributes().windowAnimations = (R.style.DialogTheme);
        alertDialog.setView(promptsView);
        alertDialog.show();
    }




    @SuppressLint("StaticFieldLeak")
    private class VerifyWifiBackground extends AsyncTask<String, Void, String> {
        int WifiRetries = 0;

        @Override
        protected String doInBackground(String... params) {
            wifiManager = (WifiManager) getApplicationContext().getSystemService(Context.WIFI_SERVICE);
            assert wifiManager != null;
            wifiInfo = wifiManager.getConnectionInfo();
            wifiManager.disconnect();
            list = wifiManager.getConfiguredNetworks();
            for( WifiConfiguration i : list ) {
                wifiManager.removeNetwork(i.networkId);
                wifiManager.saveConfiguration();
            }
            wifiConfiguration = new WifiConfiguration();
            switch( NetworkTools.customEncrypt)
            {
                case '1': case '2': case '3':
                    wifiConfiguration.preSharedKey = String.format("\"%s\"",  NetworkTools.customPASS);
                    break;
                case '4':
                    wifiConfiguration.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.NONE);
                    break;
            }
            wifiConfiguration.SSID =  NetworkTools.customSSID;
            assert wifiManager != null;
            int netId = wifiManager.addNetwork(wifiConfiguration);
            wifiManager.enableNetwork(netId, true);
            wifiManager.reconnect();
            while(WifiRetries < WIFI_CONNECT_RETRIES) {
                if (networkTools.isNetworkAvailable(getApplicationContext()) && (!Objects.equals(wifiInfo.getSSID(),""))) break;
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                WifiRetries++;
            }
            return "Executed";
        }

        @Override
        protected void onPostExecute(String result) {
            if (WifiRetries < 5) {
                if (progressDialog.isShowing()) progressDialog.dismiss();
                jumpToNextIntent(true);
            }
            else {
                alertDialog = new AlertDialog.Builder(MainMenu.this).create();
                alertDialog.setTitle("WiFi connection status");
                alertDialog.setMessage(Html.fromHtml("<font color='#A6CCFF'>Cannot connect to the selected network...</font>"));
                alertDialog.setButton(AlertDialog.BUTTON_NEUTRAL, "OK",
                        (dialog, which) -> {
                            dialog.dismiss();
                            jumpToNextIntent(false);
                        });
                Objects.requireNonNull(alertDialog.getWindow()).getAttributes().windowAnimations = (R.style.DialogTheme);
                alertDialog.getWindow().setBackgroundDrawableResource(R.color.colorx);
                alertDialog.show();
            }
        }


        @Override
        protected void onPreExecute() {
            showProgress("Verifying...");
        }

        @Override
        protected void onProgressUpdate(Void... values) {
        }
    }


@SuppressLint("SetTextI18n")
public void getNetworkPasskey()
{
    // Alert Message Block
    layoutInflater = LayoutInflater.from(this);
    @SuppressLint("InflateParams") View promptsView = layoutInflater.inflate(R.layout.prompts, null);
    AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(MainMenu.this);
    alertDialogBuilder.setTitle("Connect to the WiFi network, by pressing " + '\"' + "OK" + '\"' + " button.");
    final TextView selectedSSID = promptsView.findViewById(R.id.selected_ssid);
    selectedSSID.setText("Selected network is: " +  NetworkTools.customSSID);
    final EditText userInput2 = promptsView.findViewById(R.id.editTextDialogUserInput2);
    // set dialog message
        alertDialogBuilder
                .setCancelable(false)
                .setPositiveButton("OK",
                        (dialog, id) -> {
                            NetworkTools.customPASS = userInput2.getText().toString();
                            Log.i(TAG, "ssid_result = " +  NetworkTools.customSSID);
                            Log.i(TAG, "pass_result = " +  NetworkTools.customPASS);
                            new VerifyWifiBackground().execute();
                        })
                .setNeutralButton("Scan",
                        (dialog, id) -> jumpToNextIntent(false))
                .setNegativeButton("Cancel",
                        (dialog, id) -> dialog.cancel());

        AlertDialog alertDialog = alertDialogBuilder.create();
        Objects.requireNonNull(alertDialog.getWindow()).setBackgroundDrawableResource(R.color.colorx);
        alertDialog.getWindow().getAttributes().windowAnimations = (R.style.DialogTheme);
        alertDialog.setView(promptsView);
        alertDialog.show();
}

    private boolean readCredentialsFromFile(boolean isMacRequested) {
        String ret = "";
        String fileName = "";
        if (isMacRequested) fileName = "mac.txt";
        else fileName = "wifi.txt";
        try {
            InputStream inputStream = this.openFileInput(fileName);
            if ( inputStream != null ) {
                InputStreamReader inputStreamReader = new InputStreamReader(inputStream);
                BufferedReader bufferedReader = new BufferedReader(inputStreamReader);
                String receiveString = "";
                StringBuilder stringBuilder = new StringBuilder();
                while ( (receiveString = bufferedReader.readLine()) != null ) {
                    Log.i(TAG, "Received string: " + receiveString);
                    if (fileName.equals("mac.txt")) NetworkTools.deviceMAC = receiveString;
                    else {
                        if (receiveString.contains("SSID:"))
                            NetworkTools.customSSID = receiveString.substring(5);
                        else if (receiveString.contains("PASS:"))
                            NetworkTools.customPASS = receiveString.substring(5);
                        else if (receiveString.contains("ENCR:")) {
                            char[] rArr = receiveString.toCharArray();
                            NetworkTools.customEncrypt = rArr[5];
                        }
                    }
                    stringBuilder.append(receiveString);
                }
                inputStream.close();
                ret = stringBuilder.toString();
            }
        }
        catch (FileNotFoundException e) {
            Log.e("login activity", "File not found: " + e.toString());
            alertDialog = new AlertDialog.Builder(MainMenu.this).create();
            alertDialog.setTitle("WiFi connection status");
            alertDialog.setMessage(Html.fromHtml("<font color='#A6CCFF'>There is no stored Wi-Fi connection!</font>"));
            alertDialog.setButton(AlertDialog.BUTTON_NEUTRAL, "OK",
                    (dialog, which) -> {
                        dialog.dismiss();
                    });
            Objects.requireNonNull(alertDialog.getWindow()).getAttributes().windowAnimations = (R.style.DialogTheme);
            alertDialog.getWindow().setBackgroundDrawableResource(R.color.colorx);
            alertDialog.show();
            return false;

        } catch (IOException e) {
            Log.e("login activity", "Can not read file: " + e.toString());
            Log.e("login activity", "File not found: " + e.toString());
            alertDialog = new AlertDialog.Builder(MainMenu.this).create();
            alertDialog.setTitle("WiFi connection status");
            alertDialog.setMessage(Html.fromHtml("<font color='#A6CCFF'>Could not open file!</font>"));
            alertDialog.setButton(AlertDialog.BUTTON_NEUTRAL, "OK",
                    (dialog, which) -> {
                        dialog.dismiss();
                    });
            Objects.requireNonNull(alertDialog.getWindow()).getAttributes().windowAnimations = (R.style.DialogTheme);
            alertDialog.getWindow().setBackgroundDrawableResource(R.color.colorx);
            alertDialog.show();
            return false;
        }
        return true;
    }
}

