/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     Ji XiangLong <jixianglong@uniontech.com>
 *
 * Maintainer: WangYu <wangyu@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef EXPANSIONPANEL_H
#define EXPANSIONPANEL_H

#include "toolbutton.h"

#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <DBlurEffectWidget>
#include <DToolButton>
#include <DWidget>
#include <DGuiApplicationHelper>
#include <QMap>

#include "albumgloabl.h"

class ExpansionPanel : public DBlurEffectWidget
{
    Q_OBJECT
public:
    //所选数据
    struct FilteData {
        QIcon icon_l_light = QIcon();//左边图标浅
        QIcon icon_l_dark = QIcon(); //左边图标深
        QIcon icon_r_light = QIcon();//右边图标浅
        QIcon icon_r_dark = QIcon(); //右边图标深
        QString icon_r_path;
        QString text;
        ItemType type = ItemType::ItemTypeNull;
    };

public:
    /**
      * @brief ExpansionPanel 构造函数
      * @param parent
      */
    explicit ExpansionPanel(QWidget *parent = nullptr);

    /**
      * @brief getUngroupButton 主题颜色改变
      * @param parent
      */
    void changeTheme(Dtk::Gui::DGuiApplicationHelper::ColorType themeType);

    //动态添加按钮
    void addNewButton(FilteData &data);

signals:
    /**
     * @description: signalItemGroup 组合框选图元信号
    */
    void signalItemGroup();


    /**
     * @description: signalItemGroup 释放组合框选图元信号
    */
    void signalItemgUngroup();

    void currentItemChanged(FilteData &data);

protected:
    void focusOutEvent(QFocusEvent *e) override;

private slots:
    void onButtonClicked(FilteData data);

private:
    //阴影背景
    QGraphicsDropShadowEffect *shadow_effect = nullptr;
    //内置按钮组
    QMap<int, ToolButton *> buttons;
    //按钮数量
    int buttonCount = 0;
    //主图层
    QVBoxLayout *layout;
};
Q_DECLARE_METATYPE(ExpansionPanel::FilteData)
#endif // EXPANSIONPANEL_H
