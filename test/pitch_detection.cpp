/*=============================================================================
   Copyright (c) 2014-2018 Joel de Guzman. All rights reserved.

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#include <q/literals.hpp>
#include <q/sfx.hpp>
#include <q/pitch_detector.hpp>

#include <vector>
#include <iostream>
#include <boost/detail/lightweight_test.hpp>

#include "notes.hpp"

namespace q = cycfi::q;
using namespace q::literals;
using std::fixed;

constexpr auto pi = q::pi;
constexpr auto sps = 44100;

// Set this to true if you want verbose print outs
constexpr auto verbose = false;

struct test_result
{
   float ave_error = 0.0f;
   float min_error = 100.0f;
   float max_error = 0.0f;
};

test_result process(
   std::vector<float>&& in
 , q::frequency actual_frequency
 , q::frequency lowest_freq
 , q::frequency highest_freq)
{
   std::cout << fixed << "Actual Frequency: " << double(actual_frequency) << std::endl;

   ////////////////////////////////////////////////////////////////////////////
   // Process
   q::pitch_detector<> pd{ lowest_freq, highest_freq, sps };
   auto const& bacf = pd.bacf();
   auto size = bacf.size();
   auto result = test_result{};
   auto frames = 0;

   for (auto i = 0; i != in.size(); ++i)
   {
      auto s = in[i];

      // Pitch Detection
      bool proc = pd(s);

      if (proc)
      {
         auto frequency = pd.frequency();

         auto error = 1200.0 * std::log2(frequency / double(actual_frequency));
         if (verbose)
         {
            std::cout
               << fixed
               << frequency
               << " Error: "
               << error
               << " cent(s)."
               << std::endl
            ;
         }

         result.ave_error += std::abs(error);
         ++frames;
         result.min_error = std::min<float>(result.min_error, std::abs(error));
         result.max_error = std::max<float>(result.max_error, std::abs(error));
      }
   }

   result.ave_error /= frames;
   return result;
}

struct params
{
   float _2nd_harmonic = 2;      // Second harmonic multiple
   float _3rd_harmonic = 3;      // Second harmonic multiple
   float _1st_level = 0.3;       // Fundamental level
   float _2nd_level = 0.4;       // Second harmonic level
   float _3rd_level = 0.3;       // Third harmonic level
   float _1st_offset = 0.0;      // Fundamental phase offset
   float _2nd_offset = 0.0;      // Second harmonic phase offset
   float _3rd_offset = 0.0;      // Third harmonic phase offset
};

std::vector<float>
gen_harmonics(q::frequency freq, params const& params_)
{
   auto period = double(sps / freq);
   constexpr float offset = 100;
   std::size_t buff_size = sps; // 1 second

   std::vector<float> signal(buff_size);
   for (int i = 0; i < buff_size; i++)
   {
      auto angle = (i + offset) / period;
      signal[i] += params_._1st_level
         * std::sin(2 * pi * (angle + params_._1st_offset));
      signal[i] += params_._2nd_level
         * std::sin(params_._2nd_harmonic * 2 * pi * (angle + params_._2nd_offset));
      signal[i] += params_._3rd_level
         * std::sin(params_._3rd_harmonic * 2 * pi * (angle + params_._3rd_offset));
   }
   return signal;
}

void process(
   params const& params_
 , q::frequency actual_frequency
 , q::frequency lowest_freq
 , q::frequency highest_freq
 , float ave_error_expected
 , float min_error_expected
 , float max_error_expected
)
{
   auto result = process(
      gen_harmonics(actual_frequency, params_)
    , actual_frequency, lowest_freq, highest_freq
   );

   std::cout << fixed << "Average Error: " << result.ave_error << " cent(s)." << std::endl;
   std::cout << fixed << "Min Error: " << result.min_error << " cent(s)." << std::endl;
   std::cout << fixed << "Max Error: " << result.max_error << " cent(s)." << std::endl;

   BOOST_TEST(result.ave_error < ave_error_expected);
   BOOST_TEST(result.min_error < min_error_expected);
   BOOST_TEST(result.max_error < max_error_expected);
}

void process(
   params const& params_
 , q::frequency actual_frequency
 , q::frequency lowest_freq
 , float ave_error_expected
 , float min_error_expected
 , float max_error_expected
)
{
   process(
      params_, actual_frequency, lowest_freq, lowest_freq * 4.5
    , ave_error_expected, min_error_expected, max_error_expected
   );
}

int main()
{
   using namespace notes;

   params params_;
   std::cout << "==================================================" << std::endl;
   std::cout << " Test middle C" << std::endl;
   std::cout << "==================================================" << std::endl;
   process(params_, middle_c, 200_Hz, 0.002, 0.0001, 0.003);

   std::cout << "==================================================" << std::endl;
   std::cout << " Test middle A" << std::endl;
   std::cout << "==================================================" << std::endl;
   process(params_, 440_Hz, 200_Hz, 0.006, 0.0008, 0.02);

   std::cout << "==================================================" << std::endl;
   std::cout << " Test Low E" << std::endl;
   std::cout << "==================================================" << std::endl;
   process(params_, low_e, low_e, 0.00005, 00004, 0.0002);

   std::cout << "==================================================" << std::endl;
   std::cout << " Test E 12th" << std::endl;
   std::cout << "==================================================" << std::endl;
   process(params_, low_e_12th, low_e, 0.0002, 0.00004, 0.0007);

   std::cout << "==================================================" << std::endl;
   std::cout << " Test E 24th" << std::endl;
   std::cout << "==================================================" << std::endl;
   process(params_, low_e_24th, low_e, 0.002, 0.00004, 0.005);

   std::cout << "==================================================" << std::endl;
   std::cout << " Test A" << std::endl;
   std::cout << "==================================================" << std::endl;
   process(params_, a, a, 0.000001, 0.000001, 0.000001);

   std::cout << "==================================================" << std::endl;
   std::cout << " Test A 12th" << std::endl;
   std::cout << "==================================================" << std::endl;
   process(params_, a_12th, a, 0.0002, 0.000001, 0.002);

   std::cout << "==================================================" << std::endl;
   std::cout << " Test A 24th" << std::endl;
   std::cout << "==================================================" << std::endl;
   process(params_, a_24th, a, 0.002, 0.0002, 0.02);

   std::cout << "==================================================" << std::endl;
   std::cout << " Test D" << std::endl;
   std::cout << "==================================================" << std::endl;
   process(params_, d, d, 0.0003, 0.00003, 0.0004);

   std::cout << "==================================================" << std::endl;
   std::cout << " Test D 12th" << std::endl;
   std::cout << "==================================================" << std::endl;
   process(params_, d_12th, d, 0.002, 0.00003, 0.003);

   std::cout << "==================================================" << std::endl;
   std::cout << " Test D 24th" << std::endl;
   std::cout << "==================================================" << std::endl;
   process(params_, d_24th, d, 0.006, 0.0003, 0.02);

   std::cout << "==================================================" << std::endl;
   std::cout << " Test G" << std::endl;
   std::cout << "==================================================" << std::endl;
   process(params_, g, g, 0.00007, 0.00007, 0.00007);

   std::cout << "==================================================" << std::endl;
   std::cout << " Test G 12th" << std::endl;
   std::cout << "==================================================" << std::endl;
   process(params_, g_12th, g, 0.00007, 0.00007, 0.00008);

   std::cout << "==================================================" << std::endl;
   std::cout << " Test G 24th" << std::endl;
   std::cout << "==================================================" << std::endl;
   process(params_, g_24th, g, 0.0002, 0.00007, 0.0004);

   std::cout << "==================================================" << std::endl;
   std::cout << " Test B" << std::endl;
   std::cout << "==================================================" << std::endl;
   process(params_, b, b, 0.002, 0.000003,  0.003);

   std::cout << "==================================================" << std::endl;
   std::cout << " Test B 12th" << std::endl;
   std::cout << "==================================================" << std::endl;
   process(params_, b_12th, b, 0.009, 0.0005, 0.02);

   std::cout << "==================================================" << std::endl;
   std::cout << " Test B 24th" << std::endl;
   std::cout << "==================================================" << std::endl;
   process(params_, b_24th, b, 0.02, 0.000003, 0.2);

   std::cout << "==================================================" << std::endl;
   std::cout << " Test B 24th (higher resolution)" << std::endl;
   std::cout << "==================================================" << std::endl;
   process(params_, b_24th, G[3], b_24th, 0.008, 0.000003, 0.03);

   std::cout << "==================================================" << std::endl;
   std::cout << " Test High E" << std::endl;
   std::cout << "==================================================" << std::endl;
   process(params_, high_e, high_e, 0.003, 0.00004, 0.005);

   std::cout << "==================================================" << std::endl;
   std::cout << " Test High E 12th" << std::endl;
   std::cout << "==================================================" << std::endl;
   process(params_, high_e_12th, high_e, 0.009, 0.00004, 0.03);

   std::cout << "==================================================" << std::endl;
   std::cout << " Test High E 24th" << std::endl;
   std::cout << "==================================================" << std::endl;
   process(params_, high_e_24th, high_e, 0.06, 0.002, 0.3);

   std::cout << "==================================================" << std::endl;
   std::cout << " Test High E 24th (higher resolution)" << std::endl;
   std::cout << "==================================================" << std::endl;
   process(params_, high_e_24th, G[3], high_e_24th, 0.03, 0.0002, 0.09);

   std::cout << "==================================================" << std::endl;
   std::cout << " Non-integer harmonics test" << std::endl;
   std::cout << "==================================================" << std::endl;
   params_._2nd_harmonic = 2.003;
   process(params_, low_e, low_e, 1.0, 0.4, 1.2);
   params_ = params{};

   std::cout << "==================================================" << std::endl;
   std::cout << " Phase offsets test" << std::endl;
   std::cout << "==================================================" << std::endl;
   params_._1st_offset = 0.1;
   params_._2nd_offset = 0.5;
   params_._3rd_offset = 0.4;
   process(params_, low_e, low_e, 0.0003, 0.00004, 0.001);
   params_ = params{};

   std::cout << "==================================================" << std::endl;
   std::cout << " Missing fundamental test" << std::endl;
   std::cout << "==================================================" << std::endl;
   params_._1st_level = 0.0;
   params_._2nd_level = 0.5;
   params_._3rd_level = 0.5;
   process(params_, low_e, low_e, 0.003, 0.00004, 0.008);
   params_ = params{};

   std::cout << "==================================================" << std::endl;
   return boost::report_errors();
}

