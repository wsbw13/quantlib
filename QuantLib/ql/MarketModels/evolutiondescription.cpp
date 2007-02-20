/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Marco Bianchetti
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2006 Giorgio Facchinetti
 Copyright (C) 2006 Mark Joshi

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/MarketModels/evolutiondescription.hpp>
#include <ql/Math/matrix.hpp>
#include <ql/Utilities/dataformatters.hpp>

namespace QuantLib {

    EvolutionDescription::EvolutionDescription()
    : steps_(0) {}

    EvolutionDescription::EvolutionDescription(
                     const std::vector<Time>& rateTimes,
                     const std::vector<Time>& evolutionTimes,
                     //const std::vector<Size>& numeraires,
                     const std::vector<std::pair<Size,Size> >& relevanceRates)
    : rateTimes_(rateTimes),
      evolutionTimes_(evolutionTimes.size()>0 ?
                      evolutionTimes :
                      std::vector<Time>(rateTimes.begin(), rateTimes.end()-1)),
      steps_(evolutionTimes_.size()),
      //numeraires_(numeraires),
      relevanceRates_(relevanceRates),
      rateTaus_(rateTimes_.size()-1),
      effStopTime_(evolutionTimes_.size(), rateTimes_.size()-1),
      firstAliveRate_(evolutionTimes_.size())
    {

        // Set up and check coherence of data: 
        // rate times, evolution times, numeraires, relevance rates, times.

        // Check rate times (n = rateTimes.size()-1):
        // - rateTimes must have 2 elements at least (n >= 1);
        // - rateTimes[i] are strictly increasing and non-negative.
        QL_REQUIRE(rateTimes_.size()>1,
                   "Rate times must have 2 elements at least");
        QL_REQUIRE(rateTimes_[0]>=0.0,
                   "first rate time must be non negative");
        Size i;
        for (i = 1; i<rateTimes.size(); ++i)
            QL_REQUIRE(rateTimes[i]>rateTimes[i-1],
                       "rate times must be strictly increasing");

        // Check evolution times (steps_ = evolutionTimes_.size()):
        // - evolutionTimes_ has 1 elements at least (steps_ >= 1);
        // - evolutionTimes_[i] are strictly increasing;
        // - the last evolutionTimes_ is <= the last rateTimes.
        QL_REQUIRE(steps_>0,
                   "Evolution times must have 1 elements at least");
        for (i = 1; i<steps_; ++i)
            QL_REQUIRE(evolutionTimes_[i]>evolutionTimes_[i-1],
                       "Evolution times must be strictly increasing");
        QL_REQUIRE(rateTimes.back() >= evolutionTimes_.back(),
                   "The last evolution time is past the last rate time");

        // Set up and check numeraires (n_num = numeraires.size()):
        // - if numeraires is empty, put the last ZCbond as default numeraire;
        // - numeraires are as many as evolutionTimes_ (n_num = steps_);
        // - the numeraire[i] must not have expired before the end of the step evolutionTimes_[i].
        //if (numeraires.empty()) {
        //    numeraires_ = std::vector<Size>(steps_, rateTimes_.size()-1);
        //} else {
        //    QL_REQUIRE(numeraires.size() == steps_,
        //               "Numeraires / evolutionTimes_ mismatch");
        //    for (Size i=0; i<numeraires.size()-1; i++)
        //        QL_REQUIRE(rateTimes[numeraires[i]] >= evolutionTimes_[i],
        //                   "Numeraire " << i << " expired");
        //}

        // Set up and check relevance rates (n_rr = relevanceRates.size()):
        // - if relevanceRates is empty, put as default 0 and n;
        // - relevanceRates are as many as evolutionTimes_ (n_rr = steps_).
        if (relevanceRates.empty()) {
            relevanceRates_ = std::vector<std::pair<Size,Size> >(
                                steps_, std::make_pair(0,rateTimes.size()-1));
        } else {
            QL_REQUIRE(relevanceRates.size() == steps_,
                       "relevanceRates / evolutionTimes mismatch");
        }

        // Set up and check times:
        // - set up rateTaus_
        // - set up the effective stop time for step j and rate time i as MIN{evolutionTimes_[j],rateTimes[i]}
        // - set up firstAliveRate_ as the first alive rate for each step
        for (i=0; i<rateTaus_.size(); i++)
            rateTaus_[i] = rateTimes_[i+1] - rateTimes_[i];

        Size j;
        for (j=0; j<steps_; ++j) {
            for (Size i=0; i<rateTimes_.size()-1; ++i)
                effStopTime_[j][i] =
                    std::min(evolutionTimes_[j], rateTimes_[i]);
        }

        Time currentEvolutionTime = 0.0;
        Size firstAliveRate = 0;
        for (j=0; j<steps_; ++j) {
            while (rateTimes_[firstAliveRate] <= currentEvolutionTime)
                ++firstAliveRate;
            firstAliveRate_[j] = firstAliveRate;
            currentEvolutionTime = evolutionTimes_[j];
        }
    }

    const std::vector<Time>& EvolutionDescription::rateTimes() const {
        return rateTimes_;
    }

    const std::vector<Time>& EvolutionDescription::rateTaus() const {
        return rateTaus_;
    }

    const std::vector<Time>& EvolutionDescription::evolutionTimes() const {
        return evolutionTimes_;
    }

    const Matrix& EvolutionDescription::effectiveStopTime() const {
        return effStopTime_;
    }

    const std::vector<Size>& EvolutionDescription::firstAliveRate() const {
        return firstAliveRate_;
    }

    const std::vector<std::pair<Size,Size> >& EvolutionDescription::relevanceRates() const {
        return relevanceRates_;
    }

    Size EvolutionDescription::numberOfRates() const {
        return rateTimes_.size() - 1; 
    }

    Size EvolutionDescription::numberOfSteps() const {
        return evolutionTimes_.size(); 
    }

    void checkCompatibility(const EvolutionDescription& evolution,
                            const std::vector<Size>& numeraires)
    {
        const std::vector<Time>& evolutionTimes = evolution.evolutionTimes();
        Size n = evolutionTimes.size();
        QL_REQUIRE(numeraires.size() == n,
                   "Size mismatch between numeraires (" << numeraires.size()
                   << ") and evolution times (" << n << ")");

        const std::vector<Time>& rateTimes = evolution.rateTimes();
        for (Size i=0; i<n-1; i++)
            QL_REQUIRE(rateTimes[numeraires[i]] >= evolutionTimes[i],
                       io::ordinal(i) << " step, evolution time " <<
                       evolutionTimes[i] << ": the numeraire (" << numeraires[i] <<
                       "), corresponding to rate time " <<
                       rateTimes[numeraires[i]] << ", is expired");
    }

    bool isInTerminalMeasure(const EvolutionDescription& evolution,
                             const std::vector<Size>& numeraires) {
        const std::vector<Time>& rateTimes = evolution.rateTimes();
        return *std::min_element(numeraires.begin(), numeraires.end()) ==
                                                          rateTimes.size()-1;
    }

    bool isInMoneyMarketPlusMeasure(const EvolutionDescription& evolution,
                                    const std::vector<Size>& numeraires,
                                    Size offset) {
        bool res = true;
        const std::vector<Time>& rateTimes = evolution.rateTimes();
        Size maxNumeraire=rateTimes.size()-1;
        QL_REQUIRE(offset<=maxNumeraire,
                   "offset (" << offset <<
                   ") is greater than the max allowed value for numeraire ("
                   << maxNumeraire << ")");
        const std::vector<Time>& evolutionTimes = evolution.evolutionTimes();
        for (Size i=0, j=0; i<evolutionTimes.size(); ++i) {
            while (rateTimes[j] < evolutionTimes[i])
                j++;
            res = (numeraires[i] == std::min(j+offset, maxNumeraire)) && res;
        }
        return res;
    }

    bool isInMoneyMarketMeasure(const EvolutionDescription& evolution,
                                const std::vector<Size>& numeraires) {
        return isInMoneyMarketPlusMeasure(evolution, numeraires, 0);
    }

    std::vector<Size> terminalMeasure(const EvolutionDescription& evolution)
    {
        return std::vector<Size>(evolution.evolutionTimes().size(),
                                 evolution.rateTimes().size()-1);
    }

    std::vector<Size> moneyMarketPlusMeasure(const EvolutionDescription& ev,
                                             Size offset) {
        const std::vector<Time>& rateTimes = ev.rateTimes();
        Size maxNumeraire = rateTimes.size()-1;
        QL_REQUIRE(offset<=maxNumeraire,
                   "offset (" << offset <<
                   ") is greater than the max allowed value for numeraire ("
                   << maxNumeraire << ")");

        const std::vector<Time>& evolutionTimes = ev.evolutionTimes();
        Size n = evolutionTimes.size();
        std::vector<Size> numeraires(n);
        for (Size i=0, j=0; i<n; ++i) {
            while (rateTimes[j] < evolutionTimes[i])
                j++;
            numeraires[i] = std::min(j+offset, maxNumeraire);
        }
        return numeraires;
    }

    std::vector<Size> moneyMarketMeasure(const EvolutionDescription& evol) {
        return moneyMarketPlusMeasure(evol, 0);
    }

}
