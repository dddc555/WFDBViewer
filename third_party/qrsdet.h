#ifndef HEART_RATE_CALC_H
#define HEART_RATE_CALC_H

#include <stdint.h>
#include <string.h>
#include <math.h>

namespace heartRate_module {
    struct HeartRateCalc {

#include <stdint.h>
        inline uint16_t _abs(int16_t val)
        {
            if(val > 0)
                return val;
            else
                return -val;
        }

// Filter
        int16_t QRSFilter(int16_t datum, uint8_t init)
        {
            int16_t fdatum;

            if(init) // Initialize filters
            {
                hpfilt(0, 1);
                lpfilt(0, 1);
                mvwint(0, 1);
                deriv(0, 1);
            }

            fdatum = lpfilt(datum, 0);	// Low pass filter data
            fdatum = hpfilt(fdatum, 0);	// High pass filter data
            fdatum = deriv(fdatum, 0);	// Take the derivative
            fdatum = _abs(fdatum);		// Take the absolute value
            fdatum = mvwint(fdatum, 0);	// Average over an 80ms window

            return fdatum;
        }

/*
 	y[n] = 2*y[n-1] - y[n-2] + x[n] - 2*x[t-24ms] + x[t-48ms]

	The filter delay is (LPBUFFER_LGTH / 2) - 1
*/
        int16_t lpfilt(int16_t datum, uint8_t init)
        {
            static int32_t y1 = 0, y2 = 0;
            static int16_t data[1000], ptr = 0;
            int32_t y0;
            int16_t output, halfPtr;

            if(init)
            {
                for(ptr = 0; ptr < LPBUFFER_LGTH; ++ptr)
                    data[ptr] = 0;
                y1 = y2 = 0;
                ptr = 0;
            }

            halfPtr = ptr - (LPBUFFER_LGTH / 2); // Use halfPtr to index

            if(halfPtr < 0) // to x[n-6]
                halfPtr += LPBUFFER_LGTH;

            y0 = (y1 << 1) - y2 + datum - (data[halfPtr] << 1) + data[ptr];
            y2 = y1;
            y1 = y0;
            output = y0 / ((LPBUFFER_LGTH * LPBUFFER_LGTH) / 4);
            data[ptr] = datum; // Stick most recent sample into
            if(++ptr == LPBUFFER_LGTH) // the circular buffer and update the buffer pointer
                ptr = 0;

            return output;
        }

/*
	y[n] = y[n-1] + x[n] - x[n-128 ms]
	z[n] = x[n-64 ms] - y[n];

	Filter delay is (HPBUFFER_LGTH-1)/2
*/
        int16_t hpfilt(int16_t datum, uint8_t init)
        {
            static int32_t y = 0;
            static int16_t data[1000], ptr = 0;
            int16_t z, halfPtr;

            if(init)
            {
                for(ptr = 0; ptr < HPBUFFER_LGTH; ++ptr)
                    data[ptr] = 0;
                ptr = 0;
                y = 0;
            }

            y += datum - data[ptr];
            halfPtr = ptr - (HPBUFFER_LGTH / 2);

            if(halfPtr < 0)
                halfPtr += HPBUFFER_LGTH;

            z = data[halfPtr] - (y / HPBUFFER_LGTH);
            data[ptr] = datum;

            if(++ptr == HPBUFFER_LGTH)
                ptr = 0;

            return z;
        }

/*
	y[n] = x[n] - x[n - 10ms]

	Filter delay is DERIV_LENGTH/2
*/
        int16_t deriv(int16_t x, uint8_t init)
        {
            static int16_t derBuff[1000], derI = 0;
            int16_t y;

            if(init != 0)
            {
                for(derI = 0; derI < DERIV_LENGTH; ++derI)
                    derBuff[derI] = 0;
                derI = 0;

                return 0;
            }

            y = x - derBuff[derI];
            derBuff[derI] = x;
            if(++derI == DERIV_LENGTH)
                derI = 0;

            return y;
        }

// Implements a moving window integrator
        int16_t mvwint(int16_t datum, uint8_t init)
        {
            static int32_t sum = 0;
            static int16_t data[1000], ptr = 0;
            int16_t output;

            if(init)
            {
                for(ptr = 0; ptr < WINDOW_WIDTH; ++ptr)
                    data[ptr] = 0;
                sum = 0;
                ptr = 0;
            }

            sum += datum;
            sum -= data[ptr];
            data[ptr] = datum;
            if(++ptr == WINDOW_WIDTH)
                ptr = 0;
            if((sum / WINDOW_WIDTH) > 32000)
                output = 32000;
            else
                output = sum / WINDOW_WIDTH;

            return output;
        }
int16_t SAMPLE_RATE	  =	250;	// Sample rate in Hz
int16_t MS_PER_SAMPLE =	4;

int16_t MS10	=((int16_t) (10 / MS_PER_SAMPLE));
int16_t MS25	=((int16_t) (25 / MS_PER_SAMPLE));
int16_t MS30	=((int16_t) (30 / MS_PER_SAMPLE));
int16_t MS50	=((int16_t) (50 / MS_PER_SAMPLE));
int16_t MS80	=((int16_t) (80 / MS_PER_SAMPLE));
int16_t MS95	=((int16_t) (95 / MS_PER_SAMPLE));
int16_t MS100	=((int16_t) (100 / MS_PER_SAMPLE));
int16_t MS125	=((int16_t) (125 / MS_PER_SAMPLE));
int16_t MS150	=((int16_t) (150 / MS_PER_SAMPLE));
int16_t MS160	=((int16_t) (160 / MS_PER_SAMPLE));
int16_t MS175	=((int16_t) (175 / MS_PER_SAMPLE));
int16_t MS195	=((int16_t) (195 / MS_PER_SAMPLE));
int16_t MS200	=((int16_t) (200 / MS_PER_SAMPLE));
int16_t MS220	=((int16_t) (220 / MS_PER_SAMPLE));
int16_t MS250	=((int16_t) (250 / MS_PER_SAMPLE));
int16_t MS300	=((int16_t) (300 / MS_PER_SAMPLE));
int16_t MS360	=((int16_t) (360 / MS_PER_SAMPLE));
int16_t MS450	=((int16_t) (450 / MS_PER_SAMPLE));
int16_t MS1000	=SAMPLE_RATE;
int16_t MS1500	=((int16_t) (1500 / MS_PER_SAMPLE));

int16_t DERIV_LENGTH	=MS10;
int16_t LPBUFFER_LGTH	=MS50;
int16_t HPBUFFER_LGTH	=MS125;

int16_t WINDOW_WIDTH	=MS80; // Moving window integration width
int16_t	FILTER_DELAY    =(int16_t)(((double)DERIV_LENGTH / 2) + ((double)LPBUFFER_LGTH / 2 - 1) + (((double)HPBUFFER_LGTH - 1) / 2) + PRE_BLANK);  // filter delays plus 200 ms blanking delay
int16_t DER_DELAY	    =WINDOW_WIDTH + FILTER_DELAY + MS100;	// 253

int16_t PRE_BLANK		=MS200;
int16_t MIN_PEAK_AMP	=50; // Prevents detections of peaks smaller than...



        const float TH = 0.425;
        const int16_t MEMMOVELEN = 7 * sizeof(int16_t);

        int16_t DDptr; // Buffer holding derivative data
        int16_t DDBuffer[3000];//DDBuffer[DER_DELAY]; // Buffer holding derivative data
         int16_t dly = 0;

#define MIN_HRM			39
#define MAX_HRM			197

#define MAX_HRM_COUNT	7
#define HRM_THRESHOLD	5

        uint16_t hrm_que[MAX_HRM_COUNT] = {0xFF,};

        void QRSDetect_setup_frequency(int16_t frequency){
            SAMPLE_RATE	  =	frequency;	// Sample rate in Hz
            MS_PER_SAMPLE =	1000 / frequency;

            MS10	=((int16_t) (10 / MS_PER_SAMPLE));
            MS25	=((int16_t) (25 / MS_PER_SAMPLE));
            MS30	=((int16_t) (30 / MS_PER_SAMPLE));
            MS50	=((int16_t) (50 / MS_PER_SAMPLE));
            MS80	=((int16_t) (80 / MS_PER_SAMPLE));
            MS95	=((int16_t) (95 / MS_PER_SAMPLE));
            MS100	=((int16_t) (100 / MS_PER_SAMPLE));
            MS125	=((int16_t) (125 / MS_PER_SAMPLE));
            MS150	=((int16_t) (150 / MS_PER_SAMPLE));
            MS160	=((int16_t) (160 / MS_PER_SAMPLE));
            MS175	=((int16_t) (175 / MS_PER_SAMPLE));
            MS195	=((int16_t) (195 / MS_PER_SAMPLE));
            MS200	=((int16_t) (200 / MS_PER_SAMPLE));
            MS220	=((int16_t) (220 / MS_PER_SAMPLE));
            MS250	=((int16_t) (250 / MS_PER_SAMPLE));
            MS300	=((int16_t) (300 / MS_PER_SAMPLE));
            MS360	=((int16_t) (360 / MS_PER_SAMPLE));
            MS450	=((int16_t) (450 / MS_PER_SAMPLE));
            MS1000	=SAMPLE_RATE;
            MS1500	=((int16_t) (1500 / MS_PER_SAMPLE));

            DERIV_LENGTH	=MS10;
            LPBUFFER_LGTH	=MS50;
            HPBUFFER_LGTH	=MS125;

            WINDOW_WIDTH	=MS80; // Moving window integration width
            FILTER_DELAY    =(int16_t)(((double)DERIV_LENGTH / 2) + ((double)LPBUFFER_LGTH / 2 - 1) + (((double)HPBUFFER_LGTH - 1) / 2) + PRE_BLANK);  // filter delays plus 200 ms blanking delay
            DER_DELAY	    =WINDOW_WIDTH + FILTER_DELAY + MS100;	// 253

            PRE_BLANK		=MS200;
            MIN_PEAK_AMP	=50; // Prevents detections of peaks smaller than...

        }

        int16_t QRSDetect(int16_t datum, uint8_t init)
        {
            static int16_t det_thresh, qpkcnt = 0;
            static int16_t qrsbuf[8], noise[8], rrbuf[8];
            static int16_t rsetBuff[8], rsetCount = 0;
            static int16_t nmean, qmean, rrmean;
            static int16_t count, sbpeak = 0, sbloc, sbcount = MS1500;
            static int16_t maxder, lastmax;
            static int16_t initBlank, initMax;
            static int16_t preBlankCnt, tempPeak;

            int16_t fdatum, QrsDelay = 0;
            int16_t i, newPeak, aPeak;

            if(init) // Init all buffers
            {
                for(i = 0; i < 8; ++i)
                {
                    noise[i] = 0; // Initialize noise buffer
                    rrbuf[i] = MS1000; // R-R interval buffer
                }

                qpkcnt = maxder = lastmax = count = sbpeak = 0;
                initBlank = initMax = preBlankCnt = DDptr = 0;
                sbcount = MS1500;

                QRSFilter(0, 1); // Init filters
                Peak(0, 1);

                hrm_que[0] = 0xFF;
            }

            fdatum = QRSFilter(datum, 0); // Filtered data

            // Wait until normal detector is ready before calling early detections
            aPeak = Peak(fdatum, 0);
            if(aPeak < MIN_PEAK_AMP)
                aPeak = 0;

            // Hold any peak that is detected for 200 ms in case a bigger one comes along
            // There can only be one QRS complex in any 200 ms window
            newPeak = 0;
            if(aPeak && !preBlankCnt) // If there has been no peak for 200 ms save this one and start counting
            {
                tempPeak = aPeak;
                preBlankCnt = PRE_BLANK; // MS200
            }
            else if(!aPeak && preBlankCnt) // If we have held onto a peak for 200 ms pass it on for evaluation
            {
                if(--preBlankCnt == 0)
                    newPeak = tempPeak;
            }
            else if(aPeak) // If we were holding a peak, but this ones bigger, save it and start counting to 200 ms again
            {
                if(aPeak > tempPeak)
                {
                    tempPeak = aPeak;
                    preBlankCnt = PRE_BLANK; // MS200
                }
                else if(--preBlankCnt == 0)
                    newPeak = tempPeak;
            }

            // Save derivative of raw signal for T-wave and baseline shift discrimination
            DDBuffer[DDptr] = deriv(datum, 0);
            if(++DDptr == DER_DELAY)
                DDptr = 0;

            // Initialize the qrs peak buffer with the first eight local maximum peaks detected
            if(qpkcnt < 8)
            {
                ++count;

                if(newPeak > 0) count = WINDOW_WIDTH;

                if(++initBlank == MS1000)
                {
                    initBlank = 0;
                    qrsbuf[qpkcnt] = initMax;
                    initMax = 0;
                    ++qpkcnt;

                    if(qpkcnt == 8)
                    {
                        qmean = mean(qrsbuf, 8);
                        nmean = 0;
                        rrmean = MS1000;
                        sbcount = MS1500 + MS150;
                        det_thresh = thresh(qmean, nmean);
                    }
                }

                if(newPeak > initMax)
                    initMax = newPeak;
            }
            else // Else test for a qrs
            {
                ++count;
                if(newPeak > 0)
                {
                    // Check for maximum derivative and matching minima and maxima for T-wave and baseline shift rejection
                    // Only consider this peak if it doesn't seem to be a base line shift
                    if(!BLSCheck(DDBuffer, DDptr, &maxder))
                    {
                        // Classify the beat as a QRS complex if the peak is larger than the detection threshold
                        if(newPeak > det_thresh)
                        {
                            memmove(&qrsbuf[1], qrsbuf, MEMMOVELEN);
                            qrsbuf[0] = newPeak;
                            qmean = mean(qrsbuf, 8);
                            det_thresh = thresh(qmean, nmean);
                            memmove(&rrbuf[1], rrbuf, MEMMOVELEN);
                            rrbuf[0] = count - WINDOW_WIDTH;
                            rrmean = mean(rrbuf,8);
                            sbcount = rrmean + (rrmean >> 1) + WINDOW_WIDTH;
                            count = WINDOW_WIDTH;

                            sbpeak = 0;

                            lastmax = maxder;
                            maxder = 0;
                            QrsDelay =  WINDOW_WIDTH + FILTER_DELAY;
                            initBlank = initMax = rsetCount = 0;
                        }
                        else // If a peak isn't a QRS update noise buffer and estimate. Store the peak for possible search back
                        {
                            memmove(&noise[1], noise, MEMMOVELEN);
                            noise[0] = newPeak;
                            nmean = mean(noise, 8);
                            det_thresh = thresh(qmean, nmean);

                            // Don't include early peaks (which might be T-waves) in the search back process
                            // A T-wave can mask a small following QRS
                            if((newPeak > sbpeak) && ((count - WINDOW_WIDTH) >= MS360))
                            {
                                sbpeak = newPeak;
                                sbloc = count  - WINDOW_WIDTH;
                            }
                        }
                    }
                }

                // Test for search back condition.  If a QRS is found in search back update the QRS buffer and det_thresh
                if((count > sbcount) && (sbpeak > (det_thresh >> 1)))
                {
                    memmove(&qrsbuf[1], qrsbuf, MEMMOVELEN);
                    qrsbuf[0] = sbpeak;
                    qmean = mean(qrsbuf, 8);
                    det_thresh = thresh(qmean, nmean);
                    memmove(&rrbuf[1], rrbuf, MEMMOVELEN);
                    rrbuf[0] = sbloc;
                    rrmean = mean(rrbuf, 8);
                    sbcount = rrmean + (rrmean >> 1) + WINDOW_WIDTH;
                    QrsDelay = count = count - sbloc;
                    QrsDelay += FILTER_DELAY;
                    sbpeak = 0;
                    lastmax = maxder;
                    maxder = 0;

                    initBlank = initMax = rsetCount = 0;
                }
            }

            // In the background estimate threshold to replace adaptive threshold if eight seconds elapses without a QRS detection
            if(qpkcnt == 8)
            {
                if(++initBlank == MS1000)
                {
                    initBlank = 0;
                    rsetBuff[rsetCount] = initMax;
                    initMax = 0;
                    ++rsetCount;

                    // Reset threshold if it has been 8 seconds without a detection
                    if(rsetCount == 8)
                    {
                        for(i = 0; i < 8; ++i)
                        {
                            qrsbuf[i] = rsetBuff[i];
                            noise[i] = 0;
                        }

                        qmean = mean(rsetBuff, 8);
                        nmean = 0;
                        rrmean = MS1000;
                        sbcount = MS1500 + MS150;
                        det_thresh = thresh(qmean, nmean);
                        initBlank = initMax = rsetCount = 0;
                    }
                }
                if(newPeak > initMax)
                    initMax = newPeak;
            }

            return QrsDelay;
        }

// Takes a datum as input and returns a peak height when the signal returns to half its peak height
        int16_t Peak(int16_t datum, uint8_t init)
        {
            static int16_t max = 0, timeSinceMax = 0, lastDatum;
            int16_t pk = 0;

            if(init)
                max = timeSinceMax = 0;

            if(timeSinceMax > 0)
                ++timeSinceMax;

            if((datum > lastDatum) && (datum > max))
            {
                max = datum;
                if(max > 2)
                    timeSinceMax = 1;
            }
            else if(datum < (max >> 1))
            {
                pk = max;
                max = 0;
                timeSinceMax = 0;
                dly = 0;
            }
            else if(timeSinceMax > MS95)
            {
                pk = max;
                max = 0;
                timeSinceMax = 0;
                dly = 3;
            }

            lastDatum = datum;

            return pk;
        }

// Get the mean of an array of integers. It uses a slow sort algorithm, but these arrays are small, so it hardly matters
        int16_t mean(int16_t *array, int16_t datnum)
        {
            int32_t sum;
            int16_t i;

            for(i = 0, sum = 0; i < datnum; ++i)
                sum += array[i];
            sum /= datnum;

            return sum;
        }

// Calculates the detection threshold from the qrs mean and noise mean estimates
        int16_t thresh(int16_t qmean, int16_t nmean)
        {
            int16_t thrsh, dmed;
            double temp;
            dmed = qmean - nmean;
            // thrsh = nmean + (dmed >> 2) + (dmed >> 3) + (dmed >> 4);
            temp = dmed;
            temp *= TH;
            dmed = temp;
            thrsh = nmean + dmed; // dmed * THRESHOLD

            return thrsh;
        }

// Reviews data to see if a baseline shift has occurred
// This is done by looking for both positive and negative slopes of	roughly the same magnitude in a 220 ms window
        int16_t BLSCheck(int16_t *dBuf, int16_t dbPtr, int16_t *maxder)
        {
            int16_t max, min, maxt, mint, t, x;
            max = min = 0;

            for(t = 0; t < MS220; ++t)
            {
                x = dBuf[dbPtr];

                if(x > max)
                {
                    maxt = t;
                    max = x;
                }
                else if(x < min)
                {
                    mint = t;
                    min = x;
                }

                if(++dbPtr == DER_DELAY)
                    dbPtr = 0;
            }

            *maxder = max;
            min = -min;

            // Possible beat if a maximum and minimum pair are found where the interval between them is less than 150 ms
            if((max > (min >> 3)) && (min > (max >> 3)) && (_abs(maxt - mint) < MS150))
                return 0;
            else
                return 1;
        }


        void hrm_roll(uint16_t val) // add to queue
        {
            if(val < MIN_HRM || val > MAX_HRM) return;

            // first fill the buffer
            if(hrm_que[0] == 0xFF)
            {
                for(uint8_t i = 0; i < MAX_HRM_COUNT; i++)
                    hrm_que[i] = val;

                return;
            }

            // check outlier
            int16_t delta = (int16_t)val - (int16_t)hrm_que[MAX_HRM_COUNT - 1];
            if(_abs(delta) > (HRM_THRESHOLD * 2))
            {
                val = hrm_que[MAX_HRM_COUNT - 1] + (delta > 0 ? HRM_THRESHOLD : -HRM_THRESHOLD);
            }

            // shift buffer
            for(uint8_t j = 1; j < MAX_HRM_COUNT; j++)
            {
                hrm_que[j - 1] = hrm_que[j];
            }
            hrm_que[MAX_HRM_COUNT - 1] = val;
        }

        uint16_t hrm_calc() // calc average
        {
            if(hrm_que[0] == 0xFF)
                return 0;

            // average
            uint16_t hrm_val = 0;
            for(uint8_t i = 0; i < MAX_HRM_COUNT; i++)
            {
                hrm_val += hrm_que[i];
            }

            return hrm_val /= MAX_HRM_COUNT;
        }

    };
};

#endif //HEART_RATE_CALC_H
