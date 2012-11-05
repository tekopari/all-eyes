/*
 * Copyright (C) <2012> <Blair Wolfinger, Ravi Jagannathan, Thomas Pari, Todd Chu>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Original Author: Thomas Pari
 */

package org.tbrt.ae;

import java.util.Calendar;
import java.util.TimeZone;

public class AeMessageStatistics {
    
    private Calendar firstReportedDTTM; // the date/time the event was first reported
    private Calendar lastReportedDTTM;  // the date/time the event was last reported
    private long   timesReported;     // the number of times the event was reported
    private String message;           // the message in text format

    //=======================================================================
    // The AE Message constructor 
    //=======================================================================
    private AeMessageStatistics() {
        firstReportedDTTM = getCurrentDttm();
        lastReportedDTTM  = firstReportedDTTM;
        timesReported = 0;
        message = "";
    }
    
    //=======================================================================
    // The constructor that we will use to initialize this class
    //=======================================================================
    public AeMessageStatistics(String msg) {
        firstReportedDTTM = getCurrentDttm();
        lastReportedDTTM  = firstReportedDTTM;
        timesReported = 1;
        message = msg;
    }

    //=======================================================================
    // Get the string that represents the current date/time
    //=======================================================================
    private Calendar getCurrentDttm() {
        return Calendar.getInstance(TimeZone.getTimeZone("GMT"));
    }
    
    //=======================================================================
    // The reportMessage method is used to update the message statistics 
    // when a new message arrives.
    //=======================================================================
    public void reportMessage() {
        lastReportedDTTM = getCurrentDttm();
        timesReported++;
    }
    
    //=======================================================================
    // The getFirstReportedDttm method returns the Date/time the message was
    // first reported
    //=======================================================================
    public Calendar getFirstReportedDttm() {
        return this.firstReportedDTTM;
    }
    
    //=======================================================================
    // The getLastReportedDttm method returns the Date/time the message was
    // last reported
    //=======================================================================
    public Calendar getLastReportedDttm() {
        return this.lastReportedDTTM;
    }

    //=======================================================================
    // The getTimesReported method returns number of times the message was
    // reported.
    //=======================================================================
    public long getTimesReported() {
        return this.timesReported;
    }
    
    //=======================================================================
    // The getMessage method returns the message string
    //=======================================================================
    public String getMessage() {
        return this.message;
    }
}
