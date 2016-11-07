#pragma once

#include <Wt/WAnchor>

#include "database/Share.hpp"

Wt::WAnchor* createShareDownloadAnchor(Database::Share::pointer share);
Wt::WAnchor* createShareEditAnchor(Database::Share::pointer share);

