/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 FIMAT Group
 Copyright (C) 2007, 2008, 2009 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/time/calendars/singapore.hpp>

namespace QuantLib {

    Singapore::Singapore(Market) {
        // all calendar instances share the same implementation instance
        static boost::shared_ptr<Calendar::Impl> impl(new Singapore::SgxImpl);
        impl_ = impl;
    }

    bool Singapore::SgxImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);

        if (isWeekend(w)
            // New Year's Day
            || (d == 1 && m == January)
            // Good Friday
            || (dd == em-3)
            // Labor Day
            || (d == 1 && m == May)
            // National Day
            || (d == 9 && m == August)
            || (d == 10 && m == August && y == 2009)
            // Christmas Day
            || (d == 25 && m == December)

            // Chinese New Year
            || ((d == 22 || d == 23) && m == January && y == 2004)
            || ((d == 9 || d == 10) && m == February && y == 2005)
            || ((d == 30 || d == 31) && m == January && y == 2006)
            || ((d == 19 || d == 20) && m == February && y == 2007)
            || ((d == 7 || d == 8) && m == February && y == 2008)
            || ((d == 26 || d == 27) && m == January && y == 2009)

            // Hari Raya Haji
            || ((d == 1 || d == 2) && m == February && y == 2004)
            || (d == 21 && m == January && y == 2005)
            || (d == 10 && m == January && y == 2006)
            || (d == 2 && m == January && y == 2007)
            || (d == 20 && m == December && y == 2007)
            || (d == 8 && m == December && y == 2008)
            || (d == 27 && m == November && y == 2009)

            // Vesak Poya Day
            || (d == 2 && m == June && y == 2004)
            || (d == 22 && m == May && y == 2005)
            || (d == 12 && m == May && y == 2006)
            || (d == 31 && m == May && y == 2007)
            || (d == 18 && m == May && y == 2008)
            || (d == 9 && m == May && y == 2009)

            // Deepavali
            || (d == 11 && m == November && y == 2004)
            || (d == 8 && m == November && y == 2007)
            || (d == 28 && m == October && y == 2008)
            || (d == 16 && m == November && y == 2009)

            // Diwali
            || (d == 1 && m == November && y == 2005)

            // Hari Raya Puasa
            || ((d == 14 || d == 15) && m == November && y == 2004)
            || (d == 3 && m == November && y == 2005)
            || (d == 24 && m == October && y == 2006)
            || (d == 13 && m == October && y == 2007)
            || (d == 1 && m == October && y == 2008)
            || (d == 21 && m == September && y == 2009)
            )
            return false;
        return true;
    }

}

