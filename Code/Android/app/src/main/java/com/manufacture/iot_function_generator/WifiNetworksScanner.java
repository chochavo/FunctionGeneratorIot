package com.manufacture.iot_function_generator;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AlertDialog;
import android.text.Html;
import android.util.Log;
import android.view.ContextThemeWrapper;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.TextView;
import android.widget.Toast;
import com.parameters.Constants;
import com.parameters.NetworkTools;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Objects;


public class WifiNetworksScanner extends Activity implements AdapterView.OnItemClickListener, Constants {
    final Context context = this;
    char[] encryptedArray = new char[32]; // Total 32 cells for each network in the networksList
    /* In-activity used variables */
    WifiManager wifiManager;
    ListView listView;
    List<ScanResult> scanResults;
    List<WifiConfiguration> networksList;
    BroadcastReceiver broadcastReceiver;
    SimpleAdapter adapter;
    ProgressDialog progressDialog;
    HashMap<String, String> hashMapItem;
    WifiConfiguration wifiConfig;
    AlertDialog alertDialog;
    LayoutInflater layoutInflater;

    ArrayList<HashMap<String, String>> arraylist = new ArrayList<>();
    NetworkTools networkTools = new NetworkTools();

    Button scanButton;
    Button backButton;

    boolean selectedNetworkAvailable = false;
    boolean asyncExecuted = false;
    int networksListSize = 0;

    boolean noNeedToUnregisterBroadcast = false;
    boolean initialListLoading = true;
    boolean firstScanIsProcessing = false;

    public void showProgress(String ProgMessage) {
        progressDialog = new ProgressDialog(new ContextThemeWrapper(this, android.R.style.Theme_Holo_Dialog));
        progressDialog.setMessage(ProgMessage);
        progressDialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
        progressDialog.setIndeterminate(true);
        progressDialog.setCancelable(false);
        if (progressDialog.isShowing()) progressDialog.dismiss();
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
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                    }
                }
            }
        };
        t.start();
    }

    public void clearNetworksList() {
        arraylist.clear();
        adapter.notifyDataSetChanged();
    }

    public void backToMainActivity() {
        unregisterReceiver(broadcastReceiver);
        Intent RetInt = new Intent(WifiNetworksScanner.this, MainMenu.class);
        finish();
        startActivity(RetInt);
    }

    @SuppressLint("StaticFieldLeak")
    private class ScanForNetworks extends AsyncTask<String, Void, String> {

    @Override
    protected String doInBackground(String... params) {
        while (networksListSize <= 0)
        {
            wifiManager.startScan();
            try {
                Thread.sleep(2000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        return "Executed";
    }

    @Override
    protected void onPostExecute(String result) {
        fillNetworksList();
        if (progressDialog.isShowing()) progressDialog.dismiss();
    }

    @Override
    protected void onPreExecute() {
        showProgress("Refreshing...");
        if (!firstScanIsProcessing) clearNetworksList();
    }

    @Override
    protected void onProgressUpdate(Void... values) { }

}



    @SuppressLint("ClickableViewAccessibility")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
        setContentView(R.layout.activity_wifi_connect);
        scanButton = findViewById(R.id.button1);
        scanButton.setOnTouchListener((v1, event) -> {
            switch(event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    scanButton.setBackgroundResource(R.color.colorx);
                    return true; // if you want to handle the touch event

                case MotionEvent.ACTION_UP:
                    scanButton.setBackgroundResource(R.color.buttonColor);
                    new ScanForNetworks().execute("");
                    return true; // if you want to handle the touch event
            }
            return false;
        });
        backButton = findViewById(R.id.back_button);
        backButton.setOnTouchListener((View v1, MotionEvent event) -> {
            switch(event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    //start_sequence.setBackgroundColor(0xFFFF0000);
                    backButton.setBackgroundResource(R.color.colorx);
                    // PRESSED
                    return true; // if you want to handle the touch event
                case MotionEvent.ACTION_UP:
                    //start_sequence.setBackgroundColor();
                    backButton.setBackgroundResource(R.color.buttonColor);
                    backToMainActivity();
                    // RELEASED
                    return true; // if you want to handle the touch event
            }
            return false;
        });
        listView = findViewById(R.id.list);
        listView.setFastScrollEnabled(true);

        if (Objects.requireNonNull(getIntent().getExtras()).getBoolean(("CONNECT_FLAG")))
        {
            noNeedToUnregisterBroadcast = true;
            if (saveCredentialsToFile()) clientAlertDialog(true);
            else clientAlertDialog(false);
        }
        else proceedToWifiConnectionSequence();
        firstScanIsProcessing = false;
    }

    @Override
    public void unregisterReceiver(BroadcastReceiver receiver) {
        super.unregisterReceiver(receiver);
    }

    public void onPause() {
        super.onPause();
        if (!noNeedToUnregisterBroadcast) if (progressDialog.isShowing()) progressDialog.dismiss();
    }

    @Override
    protected void onResume(){
        super.onResume();

    }
    @Override
    protected void onDestroy(){
        super.onDestroy();
        if (!noNeedToUnregisterBroadcast) if (progressDialog.isShowing()) progressDialog.dismiss();
    }

    public int dbmToPercent(int dbmIn)
    {
        int PerOut = 0;
        if (dbmIn >= -50) PerOut = 100;
        else PerOut = 2 * (100 + dbmIn);
      return PerOut;
    }

    public void fillNetworksList()
    {
        int networkListPointer = networksListSize - 1;
        networksListSize--;
        Log.i(TAG,"Current networks networksList networksArrayPointer is: " + networksListSize);
        if ((networksListSize > 0) && (initialListLoading)) if (progressDialog.isShowing()) progressDialog.dismiss();
        while (networksListSize >= 0)
        {
            hashMapItem = new HashMap<>();
            if ((scanResults.get(networkListPointer - networksListSize).frequency > 1000 && scanResults.get(networkListPointer - networksListSize).frequency < 3000)
            && (!scanResults.get(networkListPointer - networksListSize).SSID.contains("SM2")))
            {
                hashMapItem.put(ITEM_KEY_SSID, scanResults.get(networkListPointer - networksListSize).SSID);
                hashMapItem.put(ITEM_KEY_INTENSITY, dbmToPercent(scanResults.get(networkListPointer - networksListSize).level) + "%");
                arraylist.add(hashMapItem);
                runOnUiThread(() -> adapter.notifyDataSetChanged());
                String capabilitiesString = scanResults.get(networkListPointer - networksListSize).capabilities;
                Log.w(TAG,"San result capabilities: " + capabilitiesString);
                if (capabilitiesString.contains("WPA"))
                    encryptedArray[networkListPointer - networksListSize] = '2';
                else if (capabilitiesString.toLowerCase().contains("WEP"))
                    encryptedArray[networkListPointer - networksListSize] = '3';
                else encryptedArray[networkListPointer - networksListSize] = '4';
            }
            networksListSize--;
        }
    }



    public void removeSavedNetworks()
    {
        Log.v(TAG, "Removing stored WiFi networks...");
        networksList = wifiManager.getConfiguredNetworks();
        for( WifiConfiguration i : networksList) wifiManager.removeNetwork(i.networkId);
    }


    private void connectToWifiNetwork()
    {
        wifiManager = (WifiManager) getApplicationContext().getSystemService(Context.WIFI_SERVICE);
        assert wifiManager != null;
        List<WifiConfiguration> list = wifiManager.getConfiguredNetworks();
        for( WifiConfiguration i : list ) {
            wifiManager.removeNetwork(i.networkId);
            wifiManager.saveConfiguration();
        }
        wifiConfig = new WifiConfiguration();
        switch(NetworkTools.customEncrypt)
        {
            case WPA_2:
                wifiConfig.preSharedKey = String.format("\"%s\"",  NetworkTools.customPASS);
                break;
            case WPA:
                wifiConfig.preSharedKey = String.format("\"%s\"",  NetworkTools.customPASS);
                break;
            case WEP:
                wifiConfig.wepKeys[0] = "\"" +  NetworkTools.customPASS + "\"";
                wifiConfig.wepTxKeyIndex = 0;
                wifiConfig.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.NONE);
                wifiConfig.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.WEP40);
                break;
            case OPEN:
                wifiConfig.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.NONE);
                break;
        }
        wifiConfig.SSID =  NetworkTools.customSSID;
        assert wifiManager != null;
        int netId = wifiManager.addNetwork(wifiConfig);
        wifiManager.disconnect();
        wifiManager.enableNetwork(netId, true);
        wifiManager.reconnect();
        showProgress("Connecting to: " +  NetworkTools.customSSID);
        CountDownTimer mycnt = new CountDownTimer(WAIT_FOR_WIFI_TO_CONNECT, 2000 ) {
            public void onTick(long millisUntilFinished) {}
            public void onFinish() {
                cancel();
                connectToWiFi();
            }
        }.start();
    }


    @SuppressLint("StaticFieldLeak")
    private class CheckWifiNetConnection extends AsyncTask<String, Void, String>
    {
        @Override
        protected String doInBackground(String... params) {
            selectedNetworkAvailable = networkTools.isNetworkAvailable(getApplicationContext());
            asyncExecuted = true;
            return "Executed";
        }

        @Override
        protected void onPostExecute(String result) { }

        @Override
        protected void onPreExecute() { }

        @Override
        protected void onProgressUpdate(Void... values) { }
    }


    public void connectToWiFi() {
        wifiManager = (WifiManager) getApplicationContext().getSystemService(Context.WIFI_SERVICE);
        assert wifiManager != null;
        wifiManager.setWifiEnabled(true);
        new CheckWifiNetConnection().execute("");
        CountDownTimer mycnt = new CountDownTimer(WIFI_CONNECT_RETRIES * WIFI_CONNECT_STEP, WIFI_CONNECT_STEP ) {
            public void onTick(long millisUntilFinished) {
                if (asyncExecuted) {
                    if (selectedNetworkAvailable) {
                        cancel();
                        if (saveCredentialsToFile()) clientAlertDialog(true);
                        else clientAlertDialog(false);
                    }
                    asyncExecuted = false;
                    new CheckWifiNetConnection().execute("");
                }
            }
            public void onFinish() {
                if (progressDialog.isShowing()) progressDialog.dismiss();
                cancel();
                alertDialog = new AlertDialog.Builder(context).create();
                alertDialog.setMessage(Html.fromHtml("<font color='#A6CCFF'>Selected network is not available...</font>"));
                alertDialog.setButton(AlertDialog.BUTTON_NEUTRAL, "OK", (dialog, which) -> dialog.dismiss());
                Objects.requireNonNull(alertDialog.getWindow()).getAttributes().windowAnimations = (R.style.DialogTheme);
                alertDialog.getWindow().setBackgroundDrawableResource(R.color.colorx);
                alertDialog.show();

            }
        }.start();
    }

    @SuppressLint("SetTextI18n")
    public void proceedToWifiConnectionSequence() {
        registerReceiver(broadcastReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context c, Intent intent)
            {
                scanResults = wifiManager.getScanResults();
                networksListSize = scanResults.size();
            }
        }, new IntentFilter(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION));
        wifiManager = (WifiManager) getApplicationContext().getSystemService(Context.WIFI_SERVICE);
        assert wifiManager != null;
        removeSavedNetworks();
        if (!wifiManager.isWifiEnabled()) wifiManager.setWifiEnabled(true);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M && checkSelfPermission(Manifest.permission.ACCESS_COARSE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
            requestPermissions(new String[]{Manifest.permission.ACCESS_COARSE_LOCATION}, PERMISSIONS_REQUEST_CODE_ACCESS_COARSE_LOCATION);
        } else {
            String[] entries = {ITEM_KEY_SSID, ITEM_KEY_INTENSITY};
            int[] views = {R.id.list_value, R.id.percent_value};
            this.adapter = new SimpleAdapter(WifiNetworksScanner.this, arraylist, R.layout.row, entries, views);
            listView.setAdapter(this.adapter);
            listView.setOnItemClickListener((parent, view, position, id) -> {
                listView.setOnFocusChangeListener((v, hasFocus) -> {
                    if (hasFocus) {
                        listView.setItemChecked(position, true);
                        view.setBackgroundColor(Color.BLUE);
                    }
                });
                layoutInflater = LayoutInflater.from(context);
                @SuppressLint("InflateParams") View promptsView = layoutInflater.inflate(R.layout.prompts, null);
                AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(context);
                alertDialogBuilder.setView(promptsView);
                final TextView selectedSSID = (TextView) promptsView.findViewById(R.id.selected_ssid);
                NetworkTools.customSSID = arraylist.get(position).get(ITEM_KEY_SSID);
                Log.w(TAG, "Selected SSID is: " + arraylist.get(position).get(ITEM_KEY_SSID));
                if (encryptedArray[position] != 0) NetworkTools.customEncrypt = encryptedArray[position];
                else {
                    Log.e(TAG,"Encrypted Array of security keys returned null!");
                    Log.w(TAG,"Current encryptedArray[position]: " + encryptedArray[position] );
                }
                Log.i(TAG, "Total encryptedArray is: " + Arrays.toString(encryptedArray));
                alertDialogBuilder.setTitle("Connect to the WiFi network, by pressing " + '\"' + "OK" + '\"' + " button.");
                selectedSSID.setText("Selected network is: " + NetworkTools.customSSID);
                final EditText userInput2 = promptsView.findViewById(R.id.editTextDialogUserInput2);
                Log.i(TAG, "Encrypt Type is :" + NetworkTools.customEncrypt);
                if (NetworkTools.customEncrypt == OPEN) connectToWifiNetwork();
                else {
                    alertDialogBuilder
                            .setCancelable(false)
                            .setPositiveButton("OK",
                                    (dialog, id1) -> {
                                        boolean isPasskeyInvalid = false;
                                        NetworkTools.customPASS = userInput2.getText().toString();
                                        if ((NetworkTools.customEncrypt == WPA || NetworkTools.customEncrypt == WPA_2) && NetworkTools.customPASS.length() < 8)
                                            isPasskeyInvalid = true;
                                        else if (NetworkTools.customEncrypt == WEP && NetworkTools.customPASS.length() < 5)
                                            isPasskeyInvalid = true;
                                        Log.i(TAG, "ssid_result = " + NetworkTools.customSSID);
                                        Log.i(TAG, "pass_result = " + NetworkTools.customPASS);
                                        if (isPasskeyInvalid) {
                                            if (NetworkTools.customEncrypt == WPA || NetworkTools.customEncrypt == WPA_2) showAlertDialog("Minimum passkey length is 8 characters");
                                            else if (NetworkTools.customEncrypt == WEP) showAlertDialog("Minimum passkey length is 5 characters");
                                        } else {
                                            if (progressDialog.isShowing()) progressDialog.dismiss();
                                            connectToWifiNetwork();
                                        }
                                    })
                            .setNegativeButton("Cancel",
                                    (dialog, id12) -> dialog.cancel());
                    alertDialog = alertDialogBuilder.create();
                    Objects.requireNonNull(alertDialog.getWindow()).getAttributes().windowAnimations = (R.style.DialogTheme);
                    alertDialog.getWindow().setBackgroundDrawableResource(R.color.colorx);
                    alertDialog.show();
                }
            });
        }
        firstScanIsProcessing = true;
        new ScanForNetworks().execute("");
    }

    private boolean saveCredentialsToFile() {
        String fileContents = "";
            fileContents = "SSID:" + NetworkTools.customSSID + "\n" +
                           "PASS:" + NetworkTools.customPASS + "\n" +
                           "ENCR:" + NetworkTools.customEncrypt + "\n";
            try {
                OutputStreamWriter outputStreamWriter = new OutputStreamWriter(context.openFileOutput("wifi.txt", Context.MODE_PRIVATE));
                outputStreamWriter.write(fileContents);
                outputStreamWriter.close();
            }
            catch (IOException e) {
                Log.e("Exception", "File write failed: " + e.toString());
                return false;
            }
            return true;
    }

    public void showAlertDialog(String MessageIn) {
        alertDialog = new AlertDialog.Builder(this).create();
        alertDialog.setTitle("SimpleMed+");
        alertDialog.setMessage(Html.fromHtml("<font color='#A6CCFF'>" + MessageIn + "</font>"));
        alertDialog.setMessage(MessageIn);
        alertDialog.setButton(AlertDialog.BUTTON_NEUTRAL, "OK",
                (dialog, which) -> dialog.dismiss());
        Objects.requireNonNull(alertDialog.getWindow()).getAttributes().windowAnimations = (R.style.DialogTheme);
        alertDialog.getWindow().setBackgroundDrawableResource(R.color.colorx);
        alertDialog.show();
    }

    public void clientAlertDialog(boolean isFileWritePassed) {
        alertDialog = new AlertDialog.Builder(this).create();
        if (isFileWritePassed) {
            alertDialog.setTitle("Success!");
            alertDialog.setMessage(Html.fromHtml("<font color='#A6CCFF'>" + "Wi-Fi data was saved successfully!" + "</font>"));
            alertDialog.setButton(AlertDialog.BUTTON_NEUTRAL, "OK", (dialog, which) -> initClientActivity());
        }
        else {
            alertDialog.setTitle("Oops!");
            alertDialog.setMessage(Html.fromHtml("<font color='#A6CCFF'>" + "Wi-Fi saving failed!" + "</font>"));
            alertDialog.setButton(AlertDialog.BUTTON_NEUTRAL, "OK", (dialog, which) -> backToMainActivity());
        }

        Objects.requireNonNull(alertDialog.getWindow()).getAttributes().windowAnimations = (R.style.DialogTheme);
        alertDialog.getWindow().setBackgroundDrawableResource(R.color.colorx);
        alertDialog.show();
    }

    public void initClientActivity() {
        if (!noNeedToUnregisterBroadcast) unregisterReceiver(broadcastReceiver);
        Intent RetInt = new Intent(WifiNetworksScanner.this, ClientHandler.class);
        finish();
        startActivity(RetInt);
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {}
}
