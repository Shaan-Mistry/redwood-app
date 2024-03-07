package com.example.redwood;

import static androidx.test.espresso.Espresso.onData;
import static androidx.test.espresso.Espresso.onView;
import static androidx.test.espresso.action.ViewActions.click;
import static androidx.test.espresso.assertion.ViewAssertions.matches;
import static androidx.test.espresso.matcher.ViewMatchers.isDisplayed;
import static androidx.test.espresso.matcher.ViewMatchers.withId;
import static androidx.test.espresso.matcher.ViewMatchers.withText;

import android.content.Context;

import androidx.test.ext.junit.rules.ActivityScenarioRule;
import androidx.test.platform.app.InstrumentationRegistry;
import androidx.test.ext.junit.runners.AndroidJUnit4;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.Rule;

import static org.hamcrest.CoreMatchers.anything;
import static org.hamcrest.CoreMatchers.not;
import static org.junit.Assert.*;
import java.io.FileOutputStream;
import java.io.IOException;

/**
 * Instrumented test, which will execute on an Android device.
 *
 * @see <a href="http://d.android.com/tools/testing">Testing documentation</a>
 */

@RunWith(AndroidJUnit4.class)
public class BenchmarkTest {

    public void writeToFile(String filename, String content) {
        try (FileOutputStream fos = InstrumentationRegistry.getInstrumentation().getTargetContext().openFileOutput(filename, Context.MODE_PRIVATE)) {
            fos.write(content.getBytes());
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Rule
    public ActivityScenarioRule<MainActivity> activityRule = new ActivityScenarioRule<>(MainActivity.class);


    @Test
    public void runBenchmarkTest() {

        // Open the spinner dropdown
        onView(withId(R.id.benchmarkSpinner)).perform(click());

        // Select the last item by its position
        onData(anything()).atPosition(8).perform(click());


        // Click the RUN CPU button
        onView(withId(R.id.button1)).perform(click());


        // Check if the outputTextView is displayed (indicative of a result being shown)
        onView(withId(R.id.outputTextView)).check(matches(not(withText(""))));

    }
}