package com.manufacture.iot_function_generator;

import android.annotation.SuppressLint;
import android.app.AlarmManager;
import android.app.AlertDialog;
import android.app.PendingIntent;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.DhcpInfo;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.AsyncTask;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.text.Html;
import android.util.Log;
import android.view.ContextThemeWrapper;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import com.parameters.Constants;
import com.parameters.NetworkTools;
import com.stealthcopter.networktools.ARPInfo;
import com.stealthcopter.networktools.IPTools;
import com.stealthcopter.networktools.SubnetDevices;
import com.stealthcopter.networktools.subnet.Device;

import java.io.BufferedWriter;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;
import java.net.SocketTimeoutException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Objects;

public class FuncgenMainUI extends AppCompatActivity implements Constants {

    WifiManager wifiManager;
    WifiInfo wifiInfo;
    ProgressDialog progressDialog;
    DhcpInfo dhcpInfo;
    InetAddress inetAddress;
    Socket socket;
    SocketAddress socketAddress;
    AlertDialog alertDialog;
    String messageToDevice = "";
    BufferedWriter bufferedWriter;
    byte[] readBuffer = new byte[BUFFER_LENGTH];

    Button channelAType;
    Button channelBType;
    Button bootMode;
    Button exitCommunication;
    Button getStatusButton;

    SeekBar amplitudeA;
    SeekBar amplitudeB;
    SeekBar biasA;
    SeekBar biasB;
    SeekBar lcdBrightness;
    SeekBar lcdContrast;

    TextView freqA;
    TextView freqB;

    double amplitudeAValue = 0;
    double amplitudeBValue = 0;
    double biasAValue = 0;
    double biasBValue = 0;
    char biasASign = POSITIVE;
    char biasBSign = POSITIVE;
    int lcdBrightnessValue = 0;
    int lcdContrastValue = 0;
    int frequencyAValue = 0;
    int frequencyBValue = 0;
    int batteryPercentage = 0;
    boolean acStatus = false;

    public void frequencyInputAlert(int channel) {
        // Alert Message Block
        LayoutInflater layoutInflater = LayoutInflater.from(this);
        @SuppressLint("InflateParams") View promptsView = layoutInflater.inflate(R.layout.prompts2, null);
        android.support.v7.app.AlertDialog.Builder alertDialogBuilder = new android.support.v7.app.AlertDialog.Builder(FuncgenMainUI.this);
        alertDialogBuilder.setTitle("Enter Frequency in [Hz]...");
        final EditText freqInput = promptsView.findViewById(R.id.edit_freq);
        alertDialogBuilder
                .setCancelable(false)
                .setPositiveButton("OK",
                        (dialog, id) -> {
                            if (channel == 0) {
                                String numSequence = freqInput.getText().toString();
                                frequencyAValue = Integer.parseInt(numSequence);
                                freqA.setText(numSequence);
                                sendMessageToDevice("\r\nC=0,F=\"" + numSequence + "\"\r\n");
                            }
                            else {
                                String numSequence = freqInput.getText().toString();
                                frequencyBValue = Integer.parseInt(numSequence);
                                freqB.setText(numSequence);
                                sendMessageToDevice("\r\nC=1,F=\"" + numSequence + "\"\r\n");
                            }
                        })
                .setNegativeButton("Cancel",
                        (dialog, id) -> dialog.cancel());
        android.support.v7.app.AlertDialog alertDialog = alertDialogBuilder.create();
        Objects.requireNonNull(alertDialog.getWindow()).setBackgroundDrawableResource(R.color.colorx);
        alertDialog.getWindow().getAttributes().windowAnimations = (R.style.DialogTheme);
        alertDialog.setView(promptsView);
        alertDialog.show();
    }

    void initFrequencyInput() {
        freqA = findViewById(R.id.freq_bara);
        freqB = findViewById(R.id.freq_barb);

        freqA.setOnClickListener(v -> frequencyInputAlert(0));
        freqB.setOnClickListener(v -> frequencyInputAlert(1));
    }

    //  NetworkTools.customPASS = userInput2.getText().toString();
    void initSeekBars() {
        amplitudeA = findViewById(R.id.amplitude_bara);
        amplitudeB = findViewById(R.id.amplitude_barb);
        biasA = findViewById(R.id.bias_bara);
        biasB = findViewById(R.id.bias_barb);
        lcdBrightness = findViewById(R.id.lcd_brightness);
        lcdContrast = findViewById(R.id.lcd_contrast);

        amplitudeA.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {

            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                amplitudeAValue = progress;
                amplitudeAValue /= 100;
                @SuppressLint("DefaultLocale") String ampString = String.format("%.2f", amplitudeAValue);
                Log.i(TAG,"Amplitude A:" + amplitudeAValue);
                messageToDevice = "\r\nC=0,A=\"" + ampString + "\"\r\n";
            }
            public void onStartTrackingTouch(SeekBar seekBar) {}

            public void onStopTrackingTouch(SeekBar seekBar) {
                sendMessageToDevice(messageToDevice);
                Toast.makeText(FuncgenMainUI.this, "Amplitude A is set to: " + amplitudeAValue + "[V]", Toast.LENGTH_SHORT).show();
            }
        });

        amplitudeB.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                amplitudeBValue = progress;
                amplitudeBValue /= 100;
                @SuppressLint("DefaultLocale") String ampString = String.format("%.2f", amplitudeBValue);
                Log.i(TAG,"Amplitude B:" + ampString);
                messageToDevice = "\r\nC=1,A=\"" + ampString + "\"\r\n";
            }
            public void onStartTrackingTouch(SeekBar seekBar) {}

            public void onStopTrackingTouch(SeekBar seekBar) {
                sendMessageToDevice(messageToDevice);
                Toast.makeText(FuncgenMainUI.this, "Amplitude B is set to: " + amplitudeBValue + "[V]", Toast.LENGTH_SHORT).show();
            }
        });

        biasA.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                biasAValue = progress;
                biasAValue /= 100;
                if ((biasAValue - 3.3) >= 0) {
                    biasASign = POSITIVE;
                    biasAValue -= 3.3;
                }
                else {
                    biasASign = NEGATIVE;
                    biasAValue = 3.3 - biasAValue;
                }
                @SuppressLint("DefaultLocale") String biasString = String.format("%.2f", biasAValue);
                Log.i(TAG,"Bias A:" + biasString);
                bufferString = biasString;
                messageToDevice = "\r\nC=0,B=\"" + biasASign + biasString + "\"\r\n";
            }

            public void onStartTrackingTouch(SeekBar seekBar) {}

            public void onStopTrackingTouch(SeekBar seekBar) {
                sendMessageToDevice(messageToDevice);
                Toast.makeText(FuncgenMainUI.this, "Bias A is set to: " + biasASign + bufferString + "[V]", Toast.LENGTH_SHORT).show();
            }
        });

        biasB.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                biasBValue = progress;
                biasBValue /= 100;
                if ((biasBValue - 3.3) >= 0) {
                    biasBSign = POSITIVE;
                    biasBValue -= 3.3;
                }
                else {
                    biasBSign = NEGATIVE;
                    biasBValue = 3.3 - biasBValue;
                }
                @SuppressLint("DefaultLocale") String biasString = String.format("%.2f", biasBValue);
                Log.i(TAG,"Bias B:" + biasString);
                bufferString = biasString;
                messageToDevice = "\r\nC=1,B=\"" + biasBSign + biasString + "\"\r\n";
            }

            public void onStartTrackingTouch(SeekBar seekBar) {}

            public void onStopTrackingTouch(SeekBar seekBar) {
                sendMessageToDevice(messageToDevice);
                Toast.makeText(FuncgenMainUI.this, "Bias B is set to: " + biasBSign + bufferString + "[V]", Toast.LENGTH_SHORT).show();
            }
        });

        lcdBrightness.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                lcdBrightnessValue = progress;
                messageToDevice = "\r\nLCD=B" + lcdBrightnessValue + "\r\n";
            }
            public void onStartTrackingTouch(SeekBar seekBar) {}

            public void onStopTrackingTouch(SeekBar seekBar) {
                sendMessageToDevice(messageToDevice);
                Toast.makeText(FuncgenMainUI.this, "LCD Brightness: " + lcdBrightnessValue + "%", Toast.LENGTH_SHORT).show();
            }
        });

        lcdContrast.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                lcdContrastValue = progress;
                messageToDevice = "\r\nLCD=C" + lcdContrastValue + "\r\n";
            }
            public void onStartTrackingTouch(SeekBar seekBar) {}

            public void onStopTrackingTouch(SeekBar seekBar) {
                sendMessageToDevice(messageToDevice);
                Toast.makeText(FuncgenMainUI.this, "LCD Contrast: " + lcdContrastValue + "%", Toast.LENGTH_SHORT).show();
            }
        });
    }

    String bufferString = "";

    void selectDialog(SELECTION_TYPES selectionType, int channel) {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        if (selectionType == SELECTION_TYPES.WAVEFORM) {
            builder.setTitle("Select waveform type");
            builder.setItems(WAVEFORM_TYPES, (dialog, which) -> {
                switch (which) {
                    case SINE:
                        if (channel == 0) channelAType.setText(R.string.sine);
                        else channelBType.setText(R.string.sine);
                        sendMessageToDevice("\r\nC=" + channel + ",T=\"S\"\r\n");
                        break;
                    case TRIANGLE:
                        if (channel == 0) channelAType.setText(R.string.triangle);
                        else channelBType.setText(R.string.triangle);
                        sendMessageToDevice("\r\nC=" + channel + ",T=\"T\"\r\n");
                        break;
                    case SQUARE:
                        if (channel == 0) channelAType.setText(R.string.square);
                        else channelBType.setText(R.string.square);
                        sendMessageToDevice("\r\nC=" + channel + ",T=\"Q\"\r\n");
                        break;
                    case DC:
                        if (channel == 0) channelAType.setText(R.string.dc);
                        else channelBType.setText(R.string.dc);
                        sendMessageToDevice("\r\nC=" + channel + ",T=\"D\"\r\n");
                        break;
                    case OFF:
                        if (channel == 0) channelAType.setText(R.string.off);
                        else channelBType.setText(R.string.off);
                        sendMessageToDevice("\r\nC=" + channel + ",T=\"O\"\r\n");
                        break;

                    default: break;
                }
            });
        }

        else if (selectionType == SELECTION_TYPES.GET_INF) {
            builder.setTitle("Get power status");
            builder.setItems(GET_INFO, (dialog, which) -> {
                switch (which) {
                    case VBAT:
                        sendMessageToDevice("\r\nGIVE BAT\r\n");
                        break;

                    case ACS: // cow
                        sendMessageToDevice("\r\nGIVE AC\r\n");
                        break;

                    default: break;
                }
            });
        }

        else if (selectionType == SELECTION_TYPES.BOOT_OPTION) {
            builder.setTitle("Select boot option");
            builder.setItems(BOOT_OPTIONS, (dialog, which) -> {
                switch (which) {
                    case DIRECT: // horse
                        sendMessageToDevice("\r\nBOOT=DIR\r\n");
                        Toast.makeText(FuncgenMainUI.this, "Restarting in direct communication...", Toast.LENGTH_SHORT).show();
                        backToMainMenu();
                        break;

                    case WLAN: // cow
                        sendMessageToDevice("\r\nBOOT=LAN\r\n");
                        Toast.makeText(FuncgenMainUI.this, "Restarting in WLAN communication...", Toast.LENGTH_SHORT).show();
                        backToMainMenu();
                        break;

                    case RESET: // camel
                        sendMessageToDevice("\r\nBOOT=RESET\r\n");
                        Toast.makeText(FuncgenMainUI.this, "Restarting device...", Toast.LENGTH_SHORT).show();
                        backToMainMenu();
                        break;

                    case FACTORY: // horse
                        sendMessageToDevice("\r\nBOOT=RESTORE\r\n");
                        Toast.makeText(FuncgenMainUI.this, "Restoring factory settings...", Toast.LENGTH_SHORT).show();
                        backToMainMenu();
                        break;

                    case SHDN: // cow
                        sendMessageToDevice("\r\nBOOT=SHDN\r\n");
                        Toast.makeText(FuncgenMainUI.this, "Shutting device down...", Toast.LENGTH_SHORT).show();
                        backToMainMenu();
                        break;

                    default: break;
                }
            });
        }

        AlertDialog dialog = builder.create();
        dialog.show();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_funcgen_main_ui);
        initButtons();
        initSeekBars();
        initFrequencyInput();
        new ConnectToDeviceServer().execute("");
    }



    @SuppressLint("ClickableViewAccessibility")
    void initButtons() {
        /* Buttons behavior description */
        channelAType = findViewById(R.id.channel_a_type);
        channelAType.setOnTouchListener((v1, event) -> {
            switch(event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    channelAType.setBackgroundResource(R.color.colorx);
                    return true; // if you want to handle the touch event
                case MotionEvent.ACTION_UP:
                    channelAType.setBackgroundResource(R.color.buttonColor);
                    selectDialog(SELECTION_TYPES.WAVEFORM, 0);
                    return true; // if you want to handle the touch event
            }
            return false;
        });

        channelBType = findViewById(R.id.channel_b_type);
        channelBType.setOnTouchListener((v1, event) -> {
            switch(event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    channelBType.setBackgroundResource(R.color.colorx);
                    return true; // if you want to handle the touch event
                case MotionEvent.ACTION_UP:
                    channelBType.setBackgroundResource(R.color.buttonColor);
                    selectDialog(SELECTION_TYPES.WAVEFORM, 1);
                    return true; // if you want to handle the touch event
            }
            return false;
        });

        bootMode = findViewById(R.id.boot_option);
        bootMode.setOnTouchListener((v1, event) -> {
            switch(event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    bootMode.setBackgroundResource(R.color.colorx);
                    return true; // if you want to handle the touch event
                case MotionEvent.ACTION_UP:
                    bootMode.setBackgroundResource(R.color.buttonColor);
                    selectDialog(SELECTION_TYPES.BOOT_OPTION, 0);
                    return true; // if you want to handle the touch event
            }
            return false;
        });

        getStatusButton = findViewById(R.id.get_info);
        getStatusButton.setOnTouchListener((v1, event) -> {
            switch(event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    getStatusButton.setBackgroundResource(R.color.colorx);
                    return true; // if you want to handle the touch event
                case MotionEvent.ACTION_UP:
                    getStatusButton.setBackgroundResource(R.color.buttonColor);
                    selectDialog(SELECTION_TYPES.GET_INF, 0);
                    return true; // if you want to handle the touch event
            }
            return false;
        });

        exitCommunication = findViewById(R.id.end_communication);
        exitCommunication.setOnTouchListener((v1, event) -> {
            switch(event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    exitCommunication.setBackgroundResource(R.color.colorx);
                    return true; // if you want to handle the touch event
                case MotionEvent.ACTION_UP:
                    exitCommunication.setBackgroundResource(R.color.buttonColor);
                    verifyExit();
                    return true; // if you want to handle the touch event
            }
            return false;
        });
    }

    public void showProgress(String stringIn) {
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

    public void getTcpInfo() {
        wifiManager = (WifiManager) getApplicationContext().getSystemService(Context.WIFI_SERVICE);
        assert wifiManager != null;
        wifiInfo = wifiManager.getConnectionInfo();
        Log.i(TAG, " Network SSID At TCP init: " + wifiInfo.getSSID());
        dhcpInfo = wifiManager.getDhcpInfo();
        NetworkTools.serverIP = ipv4IpConversion(dhcpInfo.gateway); // IP Gateway retrieval
        NetworkTools.s_gateway = "Default Gateway: " + NetworkTools.serverIP;
        NetworkTools.s_ipAddress = "IP Address: " + dhcpInfo.ipAddress;
        Log.i(TAG, "Retrieved Server IP is: " + NetworkTools.serverIP);
        Log.i(TAG, "Retrieved Gateway: " + dhcpInfo.gateway);
        Log.i(TAG, "Gateway is: " + NetworkTools.s_gateway);
    }

    public String ipv4IpConversion(int i) {
        return (i & 0xFF) + "." + ((i >> 8) & 0xFF) + "." + ((i >> 16) & 0xFF) + "." + ((i >> 24) & 0xFF);
    }

    private void openSocket() throws Exception {
        try {
            if (NetworkTools.connectionType == TYPE_DIRECT) inetAddress = InetAddress.getByName(NetworkTools.serverIP);
            else inetAddress = InetAddress.getByName(NetworkTools.deviceIp);
            socketAddress = new InetSocketAddress(inetAddress, SERVER_PORT);
            socket = new Socket();
            int timeoutInMs = 10 * 1000;   // 10 seconds
            socket.connect(socketAddress, timeoutInMs);
            socket.setSoTimeout(1000);
        } catch (SocketTimeoutException ste) {
            Log.i(TAG, "Timed out waiting for the socket");
            ste.printStackTrace();
            throw ste;
        }
    }


    boolean isSubnetScanFinished = false;

    private void findSubnetDevices() {
        SubnetDevices subnetDevices = new SubnetDevices();
        SubnetDevices.fromLocalAddress().setTimeOutMillis(500);
        SubnetDevices.fromLocalAddress().findDevices(new SubnetDevices.OnSubnetDeviceFound() {
            @Override
            public void onDeviceFound(Device device) {
                if (device.mac != null) {
                    if (device.mac.equals(NetworkTools.macAddressString)) {
                        Log.i(TAG, "Found mathing MAC in ARP table!");
                        subnetDevices.cancel();
                        isSubnetScanFinished = true;
                    }
                }
            }
            @Override
            public void onFinished(ArrayList<Device> devicesFound) {
                Log.i(TAG,"Finished device scanning!");
                isSubnetScanFinished = true;
            }
        });
    }


    @SuppressLint("StaticFieldLeak")
    private class ConnectToDeviceServer extends AsyncTask<String, Void, String> {
        boolean noErrorDetected = true;

        @Override
        protected String doInBackground(String... params) {
            try {
                Thread.sleep(12000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            int socketRetries = 0;
            while (socketRetries < MAX_CLIENT_RETRIES) {
                if (NetworkTools.connectionType == TYPE_DIRECT) getTcpInfo();
                else {
                    while(true) {
                        Log.i(TAG, "Starting subnet devices scan...");
                        InetAddress ipAddress = IPTools.getLocalIPv4Address();
                        Log.i(TAG, "Local IP is: " + ipAddress);
                        findSubnetDevices();
                        while (true) if (isSubnetScanFinished) break;
                        isSubnetScanFinished = false;
                        NetworkTools.deviceIp = ARPInfo.getIPAddressFromMAC(NetworkTools.deviceMAC);
                        Log.i(TAG, "MAC address at getARP state: " + NetworkTools.deviceMAC);
                        Log.i(TAG, "MAC derived IP is: " + NetworkTools.deviceIp);
                        if (NetworkTools.deviceIp == null) Log.w(TAG, "MAC - IP Retrieval FAIL!");
                        else break;
                    }
                }
                try {
                    Thread.sleep(3000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                try {
                    openSocket();
                } catch (Exception e) {
                    e.printStackTrace();
                }
                socketRetries = 0;
                while (socketRetries < MAX_CLIENT_RETRIES) {
                    if (socket.isConnected()) return "executed";
                    try {
                        Thread.sleep(3000);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                    socketRetries++;
                }
                if (socketRetries == MAX_CLIENT_RETRIES) noErrorDetected = false;
            }
            return "executed";
        }

        @Override
        protected void onPostExecute(String result) {
            if (!noErrorDetected)
                verificationAlert("Could not connect to Function Generator...", false);
            else verificationAlert("Android device connected successfully!", true);
        }

        @Override
        protected void onPreExecute() {
            showProgress("Establishing stable connection...");
        }

        @Override
        protected void onProgressUpdate(Void... values) {
        }
    }


    public void verificationAlert(String message, boolean isSocketConnected) {
        if (progressDialog.isShowing()) progressDialog.dismiss();
        Log.i(TAG, "Alert dialog was called");
        AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(this);
        if (isSocketConnected) alertDialogBuilder.setTitle("Success");
        else alertDialogBuilder.setTitle("Oops!");
        alertDialogBuilder.setCancelable(false);
        alertDialogBuilder.setMessage(Html.fromHtml("<font color='#A6CCFF'>" + message + "</font>"));
        if (isSocketConnected) alertDialogBuilder.setPositiveButton("OK", (arg0, arg1) -> alertDialog.dismiss());
        else alertDialogBuilder.setPositiveButton("OK", (arg0, arg1) -> alertDialog.dismiss());
        alertDialog = alertDialogBuilder.create();
        Objects.requireNonNull(alertDialog.getWindow()).getAttributes().windowAnimations = (R.style.DialogTheme);
        alertDialog.getWindow().setBackgroundDrawableResource(R.color.colorx);
        alertDialog.show();
    }

    public void verifyExit() {
        if (progressDialog.isShowing()) progressDialog.dismiss();
        Log.i(TAG, "Alert dialog was called");
        AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(this);
        alertDialogBuilder.setTitle("Shutdown");
        alertDialogBuilder.setCancelable(false);
        alertDialogBuilder.setMessage(Html.fromHtml("<font color='#A6CCFF'>" + "Are you sure, you want to exit to main menu?" + "</font>"));
        alertDialogBuilder.setNegativeButton("CANCEL", (arg0, arg1) -> alertDialog.dismiss());
        alertDialogBuilder.setPositiveButton("OK", (arg0, arg1) -> {
            try {
                socket.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
            backToMainMenu();
        });
        alertDialog = alertDialogBuilder.create();
        Objects.requireNonNull(alertDialog.getWindow()).getAttributes().windowAnimations = (R.style.DialogTheme);
        alertDialog.getWindow().setBackgroundDrawableResource(R.color.colorx);
        alertDialog.show();
    }

    public void sendMessageToDevice(String msg) {
        messageToDevice = msg;
        new ProcessMessageExchange().execute("");
    }


    public void backToMainMenu() {
        Intent mIntent = new Intent(this, MainMenu.class);
        finish();
        startActivity(mIntent);
    }


    @SuppressLint("StaticFieldLeak")
    private class ProcessMessageExchange extends AsyncTask<String, Void, String> {
        @Override
        protected String doInBackground(String... params) {
                    if (socket.isConnected()) {
                        try {
                            handleDeviceMessage();
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                    }
                    else return "failure";
                    return "executed";
            }

        @Override
        protected void onPostExecute(String result) {
            final String strBuff = new String(readBuffer);
            if (strBuff.contains("BAT")) {
                batteryPercentage = ((readBuffer[5] - '0') * 10) + (readBuffer[6] - '0');
                verificationAlert("Battery percentage: " + batteryPercentage, true);
            }
            else if (strBuff.contains("AC"))  {
                acStatus = strBuff.contains("ON");
                if (acStatus) verificationAlert("AC power connected!", true);
                else verificationAlert("AC power disconnected!", true);
            }
        }

        @Override
        protected void onPreExecute() {}

        @Override
        protected void onProgressUpdate(Void... values) {
        }
    }

    public void handleDeviceMessage() throws IOException {
        try {
            bufferedWriter = new BufferedWriter(new OutputStreamWriter(socket.getOutputStream()));
            bufferedWriter.write(messageToDevice);
            bufferedWriter.flush();
            int receivedLength = socket.getInputStream().read(readBuffer);
            Log.i(TAG, "Socket receivedLength value: " + receivedLength);
            Log.i(TAG, "Received array: " + new String(readBuffer));


        }
        catch (IOException e)
        {
            e.printStackTrace();
            throw e;
        }
    }


}