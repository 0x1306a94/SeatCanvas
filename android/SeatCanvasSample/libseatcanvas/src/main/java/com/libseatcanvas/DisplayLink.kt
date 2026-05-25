package com.libseatcanvas

import android.animation.ValueAnimator

class DisplayLink private constructor() : ValueAnimator.AnimatorUpdateListener {

    private var animator: ValueAnimator = ValueAnimator.ofFloat(0f, 1f)
    private var nativeContext: Long = 0

    init {
        animator.duration = 1000
        animator.repeatCount = ValueAnimator.INFINITE
        animator.addUpdateListener(this)
    }

    companion object {
        @JvmStatic
        private fun Create(nativeContext: Long): DisplayLink {
            val link = DisplayLink()
            link.nativeContext = nativeContext
            return link
        }
    }

    fun start() {
        animator.start()
    }

    fun stop() {
        animator.cancel()
    }

    override fun onAnimationUpdate(animation: ValueAnimator) {
        nativeUpdate(nativeContext)
    }

    private external fun nativeUpdate(nativeContext: Long)
}