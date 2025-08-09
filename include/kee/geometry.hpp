#pragma once

namespace kee {

class pos
{
public:
    enum class type;

    pos() = default;
    pos(pos::type pos_type, float val);

    pos::type pos_type;
    float val;
};

enum class pos::type
{
    beg,
    end,
    rel
};

class dim
{
public:
    enum class type;

    dim() = default;
    dim(dim::type dim_type, float val);

    dim::type dim_type;
    float val;
};

enum class dim::type
{
    abs,
    rel,
    /**
     * If only one of the `width` or `height` dimensions are of type `aspect`, its size is scaled from the other
     * axis, the factor stored in its `.val` attribute. If both are `aspect`, then the element fits its parent
     * as much as possible as if it were centered while respecting the provided aspect ratio.
     */
    aspect
};

class dims
{
public:
    dims() = default;
    dims(dim w, dim h);

    dim w;
    dim h;
};

class border
{
public:
    enum class type;

    border(border::type border_type, float val);

    border::type border_type;
    float val;
};

enum class border::type
{
    abs,
    rel_w,
    rel_h
};

} // namespace kee