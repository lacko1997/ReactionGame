package com.phenyl.productions.games.reactiongame

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.view.SurfaceView
import android.widget.TextView
import androidx.constraintlayout.widget.ConstraintLayout
import java.io.BufferedReader
import java.io.IOException
import java.io.InputStreamReader

class MainActivity : AppCompatActivity() {
    fun enumerateElements(resourceId:Int)
    {
        var vertexCount:Int = 0;
        var textureCount:Int = 0;
        var normalCount:Int = 0;
        var triangleCount:Int = 0;

        val reader:BufferedReader = BufferedReader(InputStreamReader(resources.openRawResource(R.raw.button_lamp_base)));
        try {
            var ln:String? = reader.readLine();
            while (ln != null)
            {
                if(ln.startsWith("v "))
                {
                    vertexCount++;
                }
                else if(ln.startsWith("vt "))
                {
                    textureCount++;
                }
                else if(ln.startsWith("vn "))
                {
                    normalCount++;
                }
                else if(ln.startsWith("f "))
                {
                    triangleCount++;
                }
                ln = reader.readLine();
            }
        }
        catch(ex: IOException)
        {

        }
        reader.close();
        beginModel(vertexCount, textureCount, normalCount, triangleCount);
    }

    fun readModelElements(resourceId:Int)
    {
        val reader:BufferedReader = BufferedReader(InputStreamReader(resources.openRawResource(R.raw.button_lamp_base)));
        try {
            var ln:String? = reader.readLine();
            while (ln != null)
            {
                addLine(ln);
                ln = reader.readLine();
            }
        }
        catch(ex: IOException)
        {

        }
        reader.close();
    }

    fun loadModel(resourceId:Int)
    {
        enumerateElements(resourceId);
        readModelElements(resourceId);
        endModel();
    }
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val renderArea:RenderActivity = RenderActivity(this);
        val layoutSurface:ConstraintLayout = findViewById<ConstraintLayout>(R.id.surface)

        loadModel(R.raw.button_lamp_base);

        renderArea.holder.addCallback(renderArea);
        layoutSurface.addView(renderArea);
    }

    companion object {
        // Used to load the 'native-lib' library on application startup.
        init {
            System.loadLibrary("native-lib")
        }
    }

    external fun beginModel(vertexCount:Int, uvCount:Int, normalCount:Int, triangleCount:Int);
    external fun addLine(ln:String);
    external fun endModel();
}