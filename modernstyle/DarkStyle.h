// DarkStyle - kept so code written for the original Qt-Frameless-Window-
// DarkStyle compiles unchanged:
//
//   QApplication::setStyle(new DarkStyle);
//
// It is a ModernStyle that is always dark. New code can use ModernStyle
// directly, which also offers a light theme and following the system.

#pragma once

#include "modernstyle.h"

class DarkStyle : public ModernStyle
{
    Q_OBJECT

public:
    DarkStyle()
        : ModernStyle(Theme::Dark)
    {
    }
    explicit DarkStyle(QStyle *baseStyle)
        : ModernStyle(Theme::Dark, baseStyle)
    {
    }
};
