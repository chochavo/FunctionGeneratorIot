package com.parameters;

public interface Constants {

    boolean TYPE_DIRECT = true;
    boolean TYPE_WLAN = false;

    char POSITIVE = '+';
    char NEGATIVE = '-';
     // Request Parameters //
     int REQUEST_LOCATION = 199;
     int PERMISSIONS_REQUEST_CODE_ACCESS_COARSE_LOCATION = 1001;
     int ZBAR_CAMERA_PERMISSION = 1;

     // Keys //
     String TAG = "key";
     String ITEM_KEY_SSID = "key_ssid";
     String ITEM_KEY_INTENSITY = "key_intensity";

     // Frequency and security //
     char WPA = '1';
     char WPA_2 = '2';
     char WEP = '3';
     char OPEN = '4';
     int FREQUENCY_LIMIT_MIN = 2000;
     int FREQUENCY_LIMIT_MAX = 3000;

    // Delay values in mS //
     int LAUNCH_SCREEN_ANIM_DURATION = 4000;
     int NETWORK_CHECK_TIME_DIFFERENCE_MS = 1000;
     int WIFI_CONNECT_TIMEOUT = 10000;
     int WIFI_CONNECT_STEP = 2000;
     int WAIT_FOR_WIFI_TO_CONNECT = 4000;
     int CAMERA_FREEZE_TIME = 2000; // In milliseconds
     int WIFI_RECONNECT_DELAY_MS = 2000;
     int SOCKET_TIMEOUT_IN_MILLIS = 3000;

     // Number of retries per sequence //
    int NUMBER_OF_NET_CHECK_RETRIES = 5;
    int MAX_CLIENT_RETRIES = 5;
    int WIFI_CONNECT_RETRIES = 2;

    // Socket //
    int SERVER_PORT = 1726;
    int BUFFER_LENGTH = 32;

    String[] WAVEFORM_TYPES = {"Sine", "Triangle", "Square", "DC", "Off", "Cancel"};
    String[] BOOT_OPTIONS = {"Direct Mode", "Wi-Fi Network", "Reset Device", "Factory Settings", "Shutdown", "Cancel"};
    String[] GET_INFO   = {"Battery Status", "AC Status", "Cancel"};
    int SINE = 0, TRIANGLE = 1, SQUARE = 2, DC = 3, OFF = 4;
    int DIRECT = 0, WLAN = 1, RESET = 2, FACTORY = 3, SHDN = 4;
    int VBAT = 0, ACS = 1;

    enum SELECTION_TYPES { WAVEFORM, BOOT_OPTION, GET_INF }


}
