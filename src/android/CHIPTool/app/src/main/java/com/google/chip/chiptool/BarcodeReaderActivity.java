package com.google.chip.chiptool;

import android.Manifest;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.util.Log;
import android.util.SparseArray;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;

import androidx.annotation.RequiresPermission;
import androidx.appcompat.app.AppCompatActivity;
import androidx.fragment.app.Fragment;
import com.google.android.gms.vision.CameraSource;
import com.google.android.gms.vision.Detector;
import com.google.android.gms.vision.barcode.Barcode;
import com.google.android.gms.vision.barcode.BarcodeDetector;

import java.io.IOException;

public class BarcodeReaderActivity extends AppCompatActivity {
    private static final String TAG = BarcodeReaderActivity.class.getSimpleName();

    public static int BARCODE_READER_ACTIVITY_REQUEST = 1;
    private static final int REQUEST_CAMERA_PERMISSION = 0;

    CameraSource cameraSource;
    CameraSourceView cameraSourceView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.barcode_activity);
        getSupportFragmentManager()
                .beginTransaction()
                .replace(R.id.barcode_frame, new BarcodeFragment(this))
                .commit();

        initializeBarcodeDetectorAndCamera();
    }

    private void initializeBarcodeDetectorAndCamera() {
        Context context = getApplicationContext();
        BarcodeDetector barcodeDetector = new BarcodeDetector.Builder(context).build();

        barcodeDetector.setProcessor(
                new Detector.Processor<Barcode>() {
                    @Override
                    public void release() {
                    }

                    @Override
                    public void receiveDetections(Detector.Detections<Barcode> detections) {
                        final SparseArray<Barcode> barcodes = detections.getDetectedItems();
                        if (barcodes.size() > 0) {
                            getSupportFragmentManager()
                                    .beginTransaction()
                                    .replace(R.id.barcode_frame, new BarcodeResultsFragment(BarcodeReaderActivity.this, barcodes))
                                    .commit();
                        }
                    }
                });

        CameraSource.Builder builder =
                new CameraSource.Builder(context, barcodeDetector)
                        .setFacing(CameraSource.CAMERA_FACING_BACK)
                        .setAutoFocusEnabled(true)
                        .setRequestedFps(30.0f);

        cameraSource = builder.build();
    }

    public static class BarcodeFragment extends Fragment {
        BarcodeReaderActivity activity;

        BarcodeFragment(BarcodeReaderActivity activity) {
            this.activity = activity;
        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container,
                                 Bundle savedInstanceState) {
            return inflater.inflate(R.xml.barcode_fragment, container, false);
        }

        @Override
        public void onViewCreated(View view, Bundle savedInstanceState) {
            super.onViewCreated(view, savedInstanceState);
            activity.cameraSourceView = (CameraSourceView) activity.findViewById(R.id.camera_view);
        }

        @RequiresPermission(Manifest.permission.CAMERA)
        @Override
        public void onResume() {
            super.onResume();
            try {
                activity.cameraSourceView.start(activity.cameraSource);
            } catch (IOException e) {
                Log.e(TAG, "Unable to start camera source.", e);
            }
        }

        @Override
        public void onPause() {
            super.onPause();
            if (activity.cameraSourceView != null) {
                activity.cameraSourceView.stop();
            }
        }

        @Override
        public void onDestroy() {
            super.onDestroy();
            if (activity.cameraSourceView != null) {
                activity.cameraSourceView.release();
            }
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        if (requestCode == REQUEST_CAMERA_PERMISSION) {
            if (permissions[0].equals(Manifest.permission.CAMERA) && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                initializeBarcodeDetectorAndCamera();
            }
        }
    }

    public static class BarcodeResultsFragment extends Fragment {
        BarcodeReaderActivity activity;
        SparseArray<Barcode> barcodes;

        BarcodeResultsFragment(BarcodeReaderActivity activity, SparseArray<Barcode> barcodes) {
            this.activity = activity;
            this.barcodes = barcodes;

            // TODO(randy.rossi): Put the raw bytes through the CHIP qr code parser
            //barcodes.get(0).rawBytes;

            // Just a test for now...
            String result = CHIPNativeBridge.getInstance().base41Encode();
            Log.e(TAG, "Called into native bridge: got " + result);

            // Display CHIP qr code info to user for manual connect to soft AP
        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container,
                                 Bundle savedInstanceState) {
            return inflater.inflate(R.xml.barcode_results_fragment, container, false);
        }

        @Override
        public void onViewCreated(View view, Bundle savedInstanceState) {
            super.onViewCreated(view, savedInstanceState);
        }
    }
}
