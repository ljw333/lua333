#include "stdafx.h"

namespace Item {
int add_item(dbproto::PlayerData* playerdata, int itemid, int itemcount) {
    LOG_LOG("add item");
    playerdata->set_uid(333);
    return 0;
}
};
