package com.matter.virtual.device.app.core.common

import androidx.lifecycle.Observer

open class Event<out T>(private val content: T) {
  var hasBeenHandled = false
    private set

  fun getContentIfNotHandled(): T? {
    return if (hasBeenHandled) {
      null
    } else {
      hasBeenHandled = true
      content
    }
  }

  fun peekContent(): T = content
}

class EventObserver<T>(private val onEventUnHandledContent: (T) -> Unit) : Observer<Event<T>> {
  override fun onChanged(event: Event<T>?) {
    event?.getContentIfNotHandled()?.let { value -> onEventUnHandledContent(value) }
  }
}
