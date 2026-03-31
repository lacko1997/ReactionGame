package com.phenyl.productions.games.reactiongame

import android.content.Context
import android.graphics.Canvas
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.view.Surface
import android.view.SurfaceHolder
import android.view.SurfaceView


class RenderActivity(context: Context?) : SurfaceView(context), SurfaceHolder.Callback
{
    init {

    }

    override fun surfaceCreated(p0: SurfaceHolder) {
        createVulkanSurface(this.holder.surface);
    }

    override fun surfaceChanged(p0: SurfaceHolder, format: Int, width: Int, height: Int)
    {
        surfaceChanged(width, height)
    }

    override fun surfaceDestroyed(p0: SurfaceHolder)
    {

    }

    override fun onDraw(canvas: Canvas?) {
        super.onDraw(canvas);
    }

    external fun drawFrame();
    external fun createVulkan();
    external fun surfaceChanged(width:Int, height:Int);
    external fun createVulkanSurface(surface:Surface);
    external fun releaseResources()
}