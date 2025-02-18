// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts

ColumnLayout {
  id: root

  Repeater {
    id: rep
    model: ListModel {
      id: skills
    }
    LimitSkillItem {
      skillname: skillname_
      usedtimes: times
    }
  }

  function update(skill, times) {
    for (let i = 0; i < rep.count; i++) {
      let data = skills.get(i);
      if (data.skillname_ === skill) {
        data.times = times;
        return;
      }
    }
    skills.append({
      skillname_: skill,
      times: times,
    });
  }
}
