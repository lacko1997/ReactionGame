package com.phenyl.productions.games.reactiongame

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.view.SurfaceView
import android.widget.TextView
import androidx.constraintlayout.widget.ConstraintLayout

class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val renderArea:RenderActivity = RenderActivity(this);
        val layoutSurface:ConstraintLayout = findViewById<ConstraintLayout>(R.id.surface)
        renderArea.holder.addCallback(renderArea);
        layoutSurface.addView(renderArea);

    }

    companion object {
        // Used to load the 'native-lib' library on application startup.
        init {
            System.loadLibrary("native-lib")
        }
    }
}