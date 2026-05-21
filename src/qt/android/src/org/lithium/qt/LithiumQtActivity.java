package org.lithium.qt;

import android.os.Bundle;
import android.system.ErrnoException;
import android.system.Os;

import org.qtproject.qt5.android.bindings.QtActivity;

import java.io.File;

public class LithiumQtActivity extends QtActivity
{
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        final File lithiumDir = new File(getFilesDir().getAbsolutePath() + "/.lithium");
        if (!lithiumDir.exists()) {
            lithiumDir.mkdir();
        }

        super.onCreate(savedInstanceState);
    }
}
