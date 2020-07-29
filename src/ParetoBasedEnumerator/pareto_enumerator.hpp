#ifndef PARETO_ENUMERATOR_HPP__
#define PARETO_ENUMERATOR_HPP__

/*
 * This is
 *   pareto_enumerator.hpp
 * that is part of the ParetoFrontEnumerationAlgorithm library, available from
 *   https://github.com/progirep/ParetoFrontEnumerationAlgorithm
 *
 * Is ia a library for enumerating all elements of a Pareto front for a
 * multi-criterial optimization problem for which all optimization objectives
 * have a finite range.
 *
 * The library and all of its files are distributed under the following license:
 *
 * -----------------------------------------------------------------------------
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2016 Ruediger Ehlers
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <list>
#include <vector>
#include <functional>

namespace paretoenumerator {

    // Main function
    void enumerateParetoFront(std::function<void(const std::vector<int> &)> callBack, std::function<bool(const std::vector<int> &)> fn, const std::vector<std::pair<int,int> > &limits);

    // Additional functions that will remain stable and may be useful for some applications
    std::list<std::vector<int> > cleanParetoFront(const std::list<std::vector<int> > &input);
}

#endif
