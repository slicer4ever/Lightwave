#ifndef LWTIMER_H
#define LWTIMER_H
#include "LWCore/LWTypes.h"
#include <cstdint>

/*! \addtogroup LWCore 
	 @{ */
/*! \brief Timer class used for timing events. this timer uses the internal std::chrono high resolution clock for calculations. */

class LWTimer{
public:
	static const int8_t Running = 1; /*!< \brief flag for if the timer is running or not. */
	static const int8_t Completed = 2; /*!< \brief flag for when the timer has passed delta ticks. */

	/*! \brief returns the current time of the clock since the epoch. */
	static uint64_t GetCurrent();

	/*! \brief returns the resolution time of the clock for 1 second. */
	static uint64_t GetResolution();

	/*!< \brief converts from the high resolution timer used internally to millisecond time. */
	static uint64_t ToMilliSecond(uint64_t Time);

	/*!< \brief converts from millisecond time to the high resolution timer. */
	static uint64_t ToHighResolution(uint64_t Time);

	/*! \brief modify's the frequency used by the timer. */
	LWTimer &SetFrequency(uint64_t Frequency);

	/*! \brief if the timer is not running, this method resumes the timer. 
		\param Current the current time as reported by the GetCurrent method.
	*/
	LWTimer &Play(uint64_t Current);

	/*! \brief if the timer is running, this method pauses the timer. */
	LWTimer &Pause(void);

	/*! \brief Forces the timer back to zero, and doesn't preserve any additional delta seconds that have accumulated. 
		\param Current the current time as reported by the GetCurrent method.
	*/
	LWTimer &ForceReset(uint64_t Current);

	/*! \brief resets the timer by frequency, preserving any additional delta seconds that have accumulated past frequency. */
	LWTimer &Reset(void);

	/*! \brief updates the current timer if the timer is running, and flags for completion once delta ticks has passed frequency. 
		\param Current the current time as reported by the GetCurrent method.
	*/
	LWTimer &Update(uint64_t Current);

	/*! \brief returns the frequency of the timer. */
	uint64_t GetFrequency(void);

	/*! \brief returns the amount of time left until delta ticks passes frequency. */
	uint64_t GetTimeLeft(void);

	/*! \brief returns the current flag of the timer, and if the timer has been completed check if the Finished bit is set. */
	uint8_t GetFlag(void);

	/*! \brief Constructor that takes the frequency the timer runs at, and a default flag which can be set to running to start the timer as soon as it's created.
		\note GetCurrent() is called with the constructor.
	*/
	LWTimer(uint64_t Frequency, uint8_t Flag);
private:
	uint64_t m_PrevTick;
	uint64_t m_Frequency;
	uint64_t m_DeltaTicks;
	uint8_t m_Flag;
};
/* @} */
#endif