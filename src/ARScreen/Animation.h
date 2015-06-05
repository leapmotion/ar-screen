#pragma once
#include <cassert>
#include <cmath>
#include <algorithm>

#include <functional>
#include <stdexcept>

//A nice namespace to collect the easing functions, with a single default implementation to start it off.
namespace EasingFunctions{
  template<typename T>
  void Linear(T& current, const T& start, const T& goal, double percent) {
    current = static_cast<T>(start + (goal-start)*percent);
  }

  template<typename T>
  void QuadInOut(T& current, const T& start, const T& goal, double percent) {
    double t = percent; //current time
    double d = 1.0f; //overall duration
    T c = goal - start; //total movement
    T b = start;

    //Taken from http://gizma.com/easing/
    if ((t /= d / 2) < 1){
      current = static_cast<T>(((c / 2)*(t*t)) + b);
    }
    else{
      current = static_cast<T>(-c / 2 * (((t-3)*(t-1)) - 1) + b);
    }
  }
}

/// A class for animated parameters.
/// Accepts an easing function, the default one being a simple linear easing.
/// The important feature is that you can precicely control how long it will take
/// the variable to reach its goal, and once the goal is set all you have to do
/// is call the update function.  Setting a new goal while the animation is in progress
/// will cause completion to reset to 0, and the value at the time of setting to be the
/// new start value.  This makes it unsuitable for chasing behaviors where Set is called
/// often, however it makes it great for fire and forget animations where you want precice
/// control of the behavior.
template <class T>
class Animated{
public:
  typedef std::function<void(T& current, const T& start, const T& goal, double percent)> EasingFunction;

  Animated(const EasingFunction& func = EasingFunctions::Linear<T>) : Animated(T()) {}
  Animated(const T& initial, double duration = 1.0, const EasingFunction& func = EasingFunctions::Linear<T>) :
    m_current(initial), m_start(initial), m_goal(initial), 
    m_duration(duration), m_completion(0.0), m_easing(func)
  {}

  //If a SetDuration function is added, make sure you handle the implied change to m_completion!
  const double& Duration() const { return m_duration; }

  const T& Current() const { return m_current; }
  const T& Goal() const { return m_goal; }

  double Completion() const { return m_completion; }
  

  void SetEasingFunction(const EasingFunction& func) {
    m_easing = func;
  }

  void Set(const T& newGoal) {
    m_goal = newGoal;
    m_start = m_current;
    m_completion = 0;
  }

  void Set(const T& newGoal, double newDuration) {
    Set(newGoal);
    m_duration = newDuration;
  }

  void SetCompletion(double percent) {
    m_completion = std::max(0.0, std::min(1.0, percent));
    m_easing(m_current, m_start, m_goal, m_completion);
  }

  void SetImmediate(const T& newGoal) {
    m_goal = newGoal;
    m_start = newGoal;
    m_current = newGoal;
    m_completion = 1.0f;
  }

  void Update(double deltaT) {
    if (!m_easing)
      throw std::runtime_error("No easing function defined");

    if (m_current == m_goal)
      return;

    m_completion += deltaT / m_duration;
    m_completion = std::max(0.0, std::min(1.0, m_completion));

    m_easing(m_current, m_start, m_goal, m_completion);
  }

private:
  T m_current;
  T m_start; //I'd really like to figure out a way to not need this.
  T m_goal;

  double m_duration;
  double m_completion; ///% complete, a value between 0.0 and 1.0.

  EasingFunction m_easing;
};

#ifdef __GNUC__
#define DEPRECATED_FUNC __attribute__((deprecated))
#elif defined(_MSC_VER)
#define DEPRECATED_FUNC __declspec(deprecated)
#else
#pragma message("WARNING: You need to implement DEPRECATED_FUNC for this compiler")
#define DEPRECATED_FUNC
#endif

// This is a simple smoothing utility class that will perform Poisson smoothing.
// The class is templated and can be used with double, float, EigenTypes::Vector3, or anything
// that overloads addition and scalar multiplication.
// When NUM_ITERATIONS is 1, the functionality is the same as exponential smoothing.
// WARNING - Due to some vagueries of Possion smoothing & floating point math,
// This is not guaranteed to ever actually reach the goal value, Zeno's Paradox style
template <class T, int _NUM_ITERATIONS = 5>
class Smoothed {
public:

  static const int NUM_ITERATIONS = _NUM_ITERATIONS;

  //No default constructor so that we can avoid nasty uninitialized memory problems
  Smoothed(const T& initialValue, float smoothStrength = 0.8f, float targetFramerate = 100.0f) :
    m_TargetFramerate(targetFramerate), m_SmoothStrength(smoothStrength) {
    SetImmediate(initialValue);
  }

  // const getters
  operator T() const { return Value(); }
  const T& Value() const { return m_Values[NUM_ITERATIONS-1]; }
  const T& Goal() const { return m_Goal; }

  // setters to control animation
  void SetGoal(const T& goal) { m_Goal = goal; }
  // sets the goal and value to the same number
  void SetImmediate(const T& value) {
    m_Goal = value;
    for (int i = 0; i<NUM_ITERATIONS; i++) {
      m_Values[i] = value;
    }
  }
  void SetSmoothStrength(float smooth) { m_SmoothStrength = smooth; }

  DEPRECATED_FUNC void SetInitialValue(const T& value) {
    for (int i=0; i<NUM_ITERATIONS; i++) {
      m_Values[i] = value;
    }
  }

  // main update function, must be called every frame
  void Update(float deltaTime) {
    const float dtExponent = deltaTime * m_TargetFramerate;
    const float smooth = std::pow(m_SmoothStrength, dtExponent);
    assert(smooth >= 0.0f && smooth <= 1.0f);
    for (int i=0; i<NUM_ITERATIONS; i++) {
      const T& prev = i == 0 ? m_Goal : m_Values[i-1];
      m_Values[i] = smooth*m_Values[i] + (1.0f-smooth)*prev;
    }
  }

private:
  T m_Values[NUM_ITERATIONS];
  T m_Goal;
  float m_TargetFramerate;
  float m_SmoothStrength;
};

// This is a polynomial interpolation function for smoothly transitioning between two values.
// The input parameter is between 0 and 1, and the return is also between 0 and 1.
// The function has zero 1st- and 2nd-order derivatives at both ends.
// See http://en.wikipedia.org/wiki/Smoothstep
template <class T>
static inline T SmootherStep(const T& x) {
  // x is blending parameter between 0 and 1
  return x*x*x*(x*(x*6 - 15) + 10);
}
