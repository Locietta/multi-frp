#pragma once

struct Unique {
    Unique() = default;
    ~Unique() = default;

    Unique(const Unique &) = delete;
    Unique &operator=(const Unique &) = delete;

    Unique(Unique &&) = default;
    Unique &operator=(Unique &&) = default;
};

struct Copy {
    Copy() = default;
    ~Copy() = default;

    Copy(const Copy &) = default;
    Copy &operator=(const Copy &) = default;
};
