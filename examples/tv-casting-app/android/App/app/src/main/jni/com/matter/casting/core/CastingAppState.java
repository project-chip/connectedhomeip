package com.matter.casting.core;

/** Represents the state of the CastingApp */
enum CastingAppState {
  UNINITIALIZED, // Before Initialize() success
  NOT_RUNNING, // After Initialize() success before Start()ing, OR After stop() success
  RUNNING, // After Start() success
}
