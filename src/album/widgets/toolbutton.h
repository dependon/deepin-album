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

#ifndef TOOLBUTTON_H
#define TOOLBUTTON_H

#include <QWidget>
#include <DPushButton>
#include <QSvgRenderer>

DWIDGET_USE_NAMESPACE

class ToolButton : public DPushButton
{
    Q_OBJECT
public:

    enum ButtonStatus
    {
        btnNormal = 0,
        btnHover = 1
    };

    /**
     * @description: ToolButton 构造函数
    */
    explicit ToolButton(QWidget *parent = nullptr);

    /**
     * @description: setText 设置控件显示的文字
    */
    void setText(const QString &text);

    /**
     * @description: setIcon 设置控件显示的图标
    */
    void setLIcon(const QIcon &icon_light, const QIcon &icon_dark);
    void setRIcon(const QIcon &icon_light, const QIcon &icon_dark);
    void setRWIcon(const QString &path);


    std::pair<QIcon, QIcon> lIcon()
    {
        return std::make_pair(m_Licon_light, m_Licon_dark);
    }

    std::pair<QIcon, QIcon> rIcon()
    {
        return std::make_pair(m_Ricon_light, m_Ricon_dark);
    }

    QString text()
    {
        return m_text;
    }

signals:
    void focusStatusChanged(bool);

protected:
    /**
     * @description: paintEvent 重新绘制控件的样式
    */
    void paintEvent(QPaintEvent *e) override;

    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;

    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    //用于绘制的文字
    QString m_text;

    //用于绘制的图片
    QIcon m_Licon_light;
    QIcon m_Licon_dark;

    QIcon m_Ricon_light;
    QIcon m_Ricon_dark;

    QIcon m_RiconWhite;

    ButtonStatus currentStatus = btnNormal;
};

#endif // TOOLBUTTON_H
