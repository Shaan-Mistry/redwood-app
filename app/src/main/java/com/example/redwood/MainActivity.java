package com.example.redwood;

import androidx.appcompat.app.AppCompatActivity;

import android.content.res.AssetManager;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.ArrayAdapter;
import android.widget.Spinner;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;


public class MainActivity extends AppCompatActivity {

    static {
        System.loadLibrary("redwood");
    }

    private TextView outputTextView;
    private Spinner benchmarkSpinner;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);


        outputTextView = findViewById(R.id.outputTextView);

        Button button1 = findViewById(R.id.button1);
        button1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String selectedBenchmark = benchmarkSpinner.getSelectedItem().toString();
                // byte[] jsonBytes = runBenchmarkCPU(selectedBenchmark);
                //String jsonString = new String(jsonBytes, StandardCharsets.UTF_8);
                String res = runBenchmarkCPU(selectedBenchmark);
                displayResult(res);
            }
        });

        Button button2 = findViewById(R.id.button2);
        button2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String result = runBenchmarkGPU(getAssets());
                displayResult(result);
            }
        });

        // Initialize the spinner and set its options
        benchmarkSpinner = findViewById(R.id.benchmarkSpinner);
        ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(
                this,
                R.array.benchmark_options,
                android.R.layout.simple_spinner_item
        );
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        benchmarkSpinner.setAdapter(adapter);
    }

    private void displayResult(String result) {
        outputTextView.setText(result);
    }

    public native String runBenchmarkCPU(String bm_name);
    public native String runBenchmarkGPU(AssetManager assetManager);
    public native void runPthreadTest();

    public void WriteToFile(String fileName, String content){
        File path = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOCUMENTS);
        File newDir = new File(path + "/" + fileName);
        try{
            if (!newDir.exists()) {
                newDir.mkdir();
            }
            FileOutputStream writer = new FileOutputStream(new File(path, fileName));
            writer.write(content.getBytes());
            writer.close();
            Log.e("TAG", "Wrote to file: "+fileName);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
