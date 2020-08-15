package com.manufacture.iot_function_generator;

import android.annotation.SuppressLint;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.net.DhcpInfo;
import android.net.wifi.SupplicantState;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.StrictMode;
import android.support.v7.app.AppCompatActivity;
import android.text.Html;
import android.util.Log;
import android.view.ContextThemeWrapper;
import android.view.WindowManager;

import com.parameters.Constants;
import com.parameters.NetworkTools;

import java.io.BufferedWriter;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;
import java.net.SocketTimeoutException;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.List;
import java.util.Objects;


public class ClientHandler extends AppCompatActivity implements Constants {

    List<WifiConfiguration> list;
    WifiConfiguration wifiConfig;
    DhcpInfo d;
    WifiManager wifiManager;
    WifiInfo wifiInfo;
    ProgressDialog progressDialog;
    Socket socket;
    BufferedWriter bufferedWriter;
    InetAddress inetAddress;
    SocketAddress socketAddress;

    public boolean connectToDeviceSSID() {
        wifiManager = (WifiManager) getApplicationContext().getSystemService(Context.WIFI_SERVICE);
        wifiConfig = new WifiConfiguration();
        wifiConfig.SSID = "\"" + "IOT_FUNCGEN" + "\"";
        wifiConfig.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.NONE);
        wifiManager.addNetwork(wifiConfig);
        Log.d(TAG, "Retrieved SSID:" + wifiConfig.SSID);
        list = wifiManager.getConfiguredNetworks();
        for (WifiConfiguration i : list) {
            if (i.SSID != null && i.SSID.equals(wifiConfig.SSID)) {
                wifiManager.disconnect();
                wifiManager.enableNetwork(i.networkId, true);
                wifiManager.reconnect();
                Log.v(TAG, "i.SSID value is: " + i.SSID);
            }
        }
        try {
            sleepSeconds(8);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        return isWifiConnected();
    }

    public void sleepSeconds(int Seconds) throws InterruptedException {
        Thread.sleep(Seconds * 1000);
    }

    @SuppressLint("StaticFieldLeak")
    private class ClientDataExchangeSequence extends AsyncTask<String, Void, String> {
        boolean noErrorDetected = true;
        @Override
        protected String doInBackground(String... params) {
            int socketRetries = 0;
            while(socketRetries < MAX_CLIENT_RETRIES) {
                if (connectToDeviceSSID()) break;
                else socketRetries++;
            }
            if (socketRetries >= MAX_CLIENT_RETRIES) noErrorDetected = false;
            else {
                getTcpInfo();
                try {
                    sleepSeconds(3);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                try {
                    openSocket();
                } catch (Exception e) {
                    e.printStackTrace();
                }
                socketRetries = 0;
//                while (socketRetries < MAX_CLIENT_RETRIES) {
//                    Log.e("SOCKET KEY!!!", "Trying to check if socket is connected");
//                    if (socket.isConnected()) break;
//                    try {
//                        sleepSeconds(3);
//                    } catch (InterruptedException e) {
//                        e.printStackTrace();
//                    }
//                    socketRetries++;
//                }
//                if (socketRetries == MAX_CLIENT_RETRIES) noErrorDetected = false;
//                else {
                    try {
                        sendWifiDetailsToDevice();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                //}
            }
            return "executed";
        }

        @Override
        protected void onPostExecute(String result) {
            if (!noErrorDetected) verificationAlert("Could not connect to device...","Oops!","OK");
            else jumpToDeviceVerification();
        }

        @Override
        protected void onPreExecute() {
            showProgress("Connecting to device...");
        }

        @Override
        protected void onProgressUpdate(Void... values) {
        }
    }


    private boolean isWifiConnected() {
        wifiManager = (WifiManager) getApplicationContext().getSystemService(Context.WIFI_SERVICE);
        assert wifiManager != null;
        wifiInfo = wifiManager.getConnectionInfo();
        if (wifiInfo.getSupplicantState() == SupplicantState.COMPLETED) {
            String currentSSID = wifiInfo.getSSID();
            Log.i(TAG, " Current SSID: " + currentSSID);
            return currentSSID.contains("IOT");
        }
        return false;
    }

    private void saveMacToFile() {
        String fileContents;
        fileContents = NetworkTools.deviceMAC;
        try {
            OutputStreamWriter outputStreamWriter = new OutputStreamWriter(this.openFileOutput("mac.txt", Context.MODE_PRIVATE));
            outputStreamWriter.write(fileContents);
            outputStreamWriter.close();
        }
        catch (IOException e) {
            Log.e("Exception", "File write failed: " + e.toString());
        }
    }

    public void returnMainActivity() {
        Intent mIntent = new Intent(ClientHandler.this, MainMenu.class);
        finish();
        startActivity(mIntent);
    }

    public void getTcpInfo() {
        wifiManager = (WifiManager) getApplicationContext().getSystemService(Context.WIFI_SERVICE);
        assert wifiManager != null;
        wifiInfo = wifiManager.getConnectionInfo();
        Log.i(TAG, " Network SSID At TCP init: " + wifiInfo.getSSID());
        d = wifiManager.getDhcpInfo();
        NetworkTools.serverIP = ipv4IpConversion(d.gateway); // IP Gateway retrieval
        NetworkTools.s_gateway = "Default Gateway: " + NetworkTools.serverIP;
        NetworkTools.s_ipAddress = "IP Address: " + d.ipAddress;
        Log.i(TAG, "Retrieved Server IP is: " +  NetworkTools.serverIP);
        Log.i(TAG, "Retrieved Gateway: " + d.gateway);
        Log.i(TAG,"Gateway is: " +  NetworkTools.s_gateway);
    }

    public void sendWifiDetailsToDevice() throws IOException {
        byte[] ackByteArray = new byte[22];
        try {
            do {
                int read_length = socket.getInputStream().read(ackByteArray);
                Log.i(TAG, "Socket read_length value: " + read_length);
                Log.i(TAG, "Acknowledge array: " + Arrays.toString(ackByteArray));
            } while (ackByteArray[0] != 'M' || ackByteArray[1] != 'A' || ackByteArray[2] != 'C');
            bufferedWriter = new BufferedWriter(new OutputStreamWriter(socket.getOutputStream()));
            bufferedWriter.write("\r\nWN=S" + NetworkTools.customSSID +
                    ",P" + NetworkTools.customPASS + ",E" + NetworkTools.customEncrypt +  "\r\n");
            bufferedWriter.flush();
        }
        catch (IOException e)
        {
            e.printStackTrace();
            throw e;
        }
        String msg = new String(ackByteArray, StandardCharsets.UTF_8);
        NetworkTools.deviceMAC = msg.substring(4);
        saveMacToFile();
    }


    public void jumpToDeviceVerification()
    {
        if (progressDialog.isShowing()) progressDialog.dismiss();
        AlertDialog alertDialog;
        if (progressDialog.isShowing()) progressDialog.dismiss();
        Log.i(TAG,"Alert dialog was called");
        AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(this);
        alertDialogBuilder.setTitle("Success!");
        alertDialogBuilder.setCancelable(false);
        alertDialogBuilder.setMessage(Html.fromHtml("<font color='#A6CCFF'>" + "Device paired successfully!\n MAC Address is:" + NetworkTools.deviceMAC + "</font>"));
        alertDialogBuilder.setPositiveButton("OK", (arg0, arg1) -> {
            Intent inx = new Intent(ClientHandler.this, MainMenu.class);
            finish();
            startActivity(inx);
        });
        alertDialog = alertDialogBuilder.create();
        Objects.requireNonNull(alertDialog.getWindow()).getAttributes().windowAnimations = (R.style.DialogTheme);
        alertDialog.getWindow().setBackgroundDrawableResource(R.color.colorx);
        alertDialog.show();

    }

    public void showProgress(String stringIn)
    {
        progressDialog = new ProgressDialog(new ContextThemeWrapper(this, android.R.style.Theme_Holo_Dialog));
        progressDialog.setMessage(stringIn);
        progressDialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
        progressDialog.setIndeterminate(true);
        progressDialog.setCancelable(false);
        progressDialog.show();
        final int totalProgressTime = 100;
        final Thread t = new Thread() {
            @Override
            public void run() {
                int jumpTime = 0;
                while(jumpTime < totalProgressTime) {
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

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
        setContentView(R.layout.activity_client);
        StrictMode.ThreadPolicy policy = new StrictMode.ThreadPolicy.Builder().permitAll().build();
        StrictMode.setThreadPolicy(policy);
        new ClientDataExchangeSequence().execute("");
    }

    public String ipv4IpConversion(int i) {
        return  (i & 0xFF) + "." +  ((i >> 8 ) & 0xFF) + "." +  ((i >> 16 ) & 0xFF) + "." + ((i >> 24 ) & 0xFF );
    }


    private void openSocket() throws Exception {
        try {
            inetAddress = InetAddress.getByName( NetworkTools.serverIP);
            socketAddress = new InetSocketAddress(inetAddress, SERVER_PORT);
            socket = new Socket();
            int timeoutInMs = 10*1000;   // 10 seconds
            socket.connect(socketAddress, timeoutInMs);
        }
        catch (SocketTimeoutException ste) {
            Log.i(TAG, "Timed out waiting for the socket");
            ste.printStackTrace();
            throw ste;
        }
    }

    public void verificationAlert(String message, String title, String buttonAlias) {
        AlertDialog alertDialog;
        if (progressDialog.isShowing()) progressDialog.dismiss();
        Log.i(TAG,"Alert dialog was called");
        AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(this);
        alertDialogBuilder.setTitle(title);
        alertDialogBuilder.setCancelable(false);
        alertDialogBuilder.setMessage(Html.fromHtml("<font color='#A6CCFF'>" + message + "</font>"));
        alertDialogBuilder.setPositiveButton(buttonAlias, (arg0, arg1) -> returnMainActivity());
        alertDialog = alertDialogBuilder.create();
        Objects.requireNonNull(alertDialog.getWindow()).getAttributes().windowAnimations = (R.style.DialogTheme);
        alertDialog.getWindow().setBackgroundDrawableResource(R.color.colorx);
        alertDialog.show();
    }
}