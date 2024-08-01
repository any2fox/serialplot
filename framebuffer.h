/*
  Copyright © 2018 Hasan Yavuz Özderya

  This file is part of serialplot.

  serialplot is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  serialplot is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with serialplot.  If not, see <http://www.gnu.org/licenses/>.
*/

// IMPORTANT NOTE: this file will be renamed to "framebuffer.h" when
// stream work is complete

#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

struct Range
{
    double start, end;
};

/// Abstract base class for all frame buffers.
class FrameBuffer
{
public:
    /// Placeholder virtual destructor
    virtual ~FrameBuffer() {};
    /// Returns size of the buffer.
    virtual unsigned size() const = 0;
    /// Returns a sample from given index.
    virtual double sample(unsigned i) const = 0;
    /// Returns minimum and maximum of the buffer values.
    virtual Range limits() const = 0;
};

/// Common base class for index and writable frame buffers
class ResizableBuffer : public FrameBuffer
{
public:
    /// Resize the buffer.
    ///
    /// @important Resizing to same value is an error.
    virtual void resize(unsigned n) = 0;
};

/// Abstract base class for writable frame buffers
class WFrameBuffer : public ResizableBuffer
{
    /// Add samples to the buffer
    virtual void addSamples(double* samples, unsigned n) = 0;
    /// Reset all data to 0
    virtual void clear() = 0;
};

/**
 * Abstract base class for X buffers.
 *
 * These buffers only contain increasing or equal (to previous) values.
 */
class XFrameBuffer : public ResizableBuffer
{
public:
    enum Index {OUT_OF_RANGE = -1};

    /**
     * Finds index for given value.
     *
     * If given value is bigger than max or smaller than minimum
     * returns `OUT_OF_RANGE`. If it's in between values, smaller
     * index is returned (not closer one).
     */
    virtual int findIndex(double value) const = 0;
};

#endif // FRAMEBUFFER_H
