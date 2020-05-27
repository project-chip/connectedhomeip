#include <assert.h>

namespace chip {

/* Pairs an object with a boolean value to determine if the object value
 * is actually valid or not.
 */
template <class T>
class Optional
{
public:
    Optional() : mHasValue(false) {}
    explicit Optional(const T & value) : mValue(value), mHasValue(true) {}

    constexpr Optional(const Optional & other) = default;
    constexpr Optional(Optional && other)      = default;
    Optional & operator=(const Optional & other) = default;

    void SetValue(const T & value)
    {
        mValue    = value;
        mHasValue = true;
    }

    void ClearValue() { mHasValue = false; }

    const T & Value() const
    {
        assert(HasValue());
        return mValue;
    }
    bool HasValue() const { return mHasValue; }

    bool operator==(const Optional & other) const
    {
        return (mHasValue == other.mHasValue) && (!mHasValue || (mValue == other.mValue));
    }

    static Optional<T> Missing() { return Optional<T>(); }
    static Optional<T> Value(const T & value) { return Optional(value); }

private:
    T mValue;
    bool mHasValue;
};

} // namespace chip
