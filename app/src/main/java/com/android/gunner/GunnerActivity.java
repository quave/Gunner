/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.gunner;

import android.app.NativeActivity;
import android.os.Bundle;
import android.util.AttributeSet;
import android.util.DisplayMetrics;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup.MarginLayoutParams;
import android.view.WindowManager.LayoutParams;
import android.widget.LinearLayout;
import android.widget.PopupWindow;
import android.widget.TextView;


public class GunnerActivity extends NativeActivity {
    GunnerActivity _activity;
    PopupWindow _popupWindow;
    TextView _labelUpRight;
    TextView _labelCenter;
    DisplayMetrics _metrics;

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
    }

    public void showUI()
    {
        if( _popupWindow != null )
            return;

        _activity = this;
        _metrics = getApplicationContext().getResources().getDisplayMetrics();

        this.runOnUiThread(new Runnable()  {
            @Override
            public void run()  {
                LayoutInflater layoutInflater
                        = (LayoutInflater)getBaseContext()
                        .getSystemService(LAYOUT_INFLATER_SERVICE);
                View popupView = layoutInflater.inflate(R.layout.widgets, null);
                _popupWindow = new PopupWindow(
                        popupView,
                        _metrics.widthPixels,
                        _metrics.heightPixels / 2);

                LinearLayout mainLayout = new LinearLayout(_activity);
                MarginLayoutParams params = new MarginLayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT);
                params.setMargins(20, 20, 20, 0);
                _activity.setContentView(mainLayout, params);

                // Show our UI over NativeActivity window
                _popupWindow.showAtLocation(mainLayout, Gravity.TOP | Gravity.LEFT, 0, 40);
                _popupWindow.update();

                _labelUpRight = (TextView)popupView.findViewById(R.id.textViewUpRight);
                _labelCenter = (TextView)popupView.findViewById(R.id.textViewCenter);
            }});
    }

    public void showCenterText(final String text)
    {
        if( _labelUpRight == null )
            return;

        _activity = this;
        this.runOnUiThread(new Runnable()  {
            @Override
            public void run()  {
                _labelCenter.setText(text);
            }});
    }

    public void showScore(final int score)
    {
        if( _labelCenter == null )
            return;

        _activity = this;
        this.runOnUiThread(new Runnable()  {
            @Override
            public void run()  {
                _labelUpRight.setText(String.format("Score %d", score));
            }});
    }
}
