/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZhangYong <zhangyong@uniontech.com>
 *
 * Maintainer: ZhangYong <ZhangYong@uniontech.com>
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
#include "thumbnaildelegate.h"
#include "utils/imageutils.h"
#include "utils/baseutils.h"
#include "application.h"
#include <QDateTime>

#include <QHBoxLayout>
#include <QLineEdit>
#include <QPainter>
#include <QPixmapCache>
#include <QStandardItemModel>
#include <QThread>
#include <QTimer>
#include <QPainterPath>
#include <QMouseEvent>
#include <QImageReader>
#include <QApplication>
#include <DFontSizeManager>
#include <DGuiApplicationHelper>
#include <timelinedatewidget.h>

#include "imageengineapi.h"
#include "imagedataservice.h"
#include "signalmanager.h"
namespace {
const QString IMAGE_DEFAULTTYPE = "All pics";
}

const int NotSupportedOrDamagedWidth = 40;      //损坏图片宽度
const int NotSupportedOrDamagedHeigh = 40;

ThumbnailDelegate::ThumbnailDelegate(DelegateType type, QObject *parent)
    : QStyledItemDelegate(parent)
    , m_imageTypeStr(IMAGE_DEFAULTTYPE)
    , selectedPixmapLight(utils::base::renderSVG(":/resources/images/other/select_active.svg", QSize(28, 28)))
    , selectedPixmapDark(utils::base::renderSVG(":/images/logo/resources/images/other/select_active_dark.svg", QSize(28, 28)))
    , m_delegatetype(type)
    , m_default(utils::base::renderSVG(":/icons/deepin/builtin/icons/light/picture_default_light.svg", QSize(60, 45)))
    , m_videoDefault(utils::base::renderSVG(":/icons/deepin/builtin/icons/light/video_default_light.svg", QSize(60, 45)))
    , m_damagePixmap(utils::image::getDamagePixmap(DApplicationHelper::instance()->themeType() == DApplicationHelper::LightType))
{
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this,
            &ThumbnailDelegate::onThemeTypeChanged);
}

void ThumbnailDelegate::setItemSize(QSize size)
{
    m_size = size;
}

void ThumbnailDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!bneedpaint) {
        return;
    }
    const DBImgInfo data = itemData(index);
    switch (data.itemType) {
    case ItemType::ItemTypeBlank: {
        //空白项，什么都不绘制
        break;
    }
    case ItemType::ItemTypeTimeLineTitle: {
        break;
    }
    case ItemType::ItemTypeImportTimeLineTitle: {
        break;
    }
    case ItemType::ItemTypePic:
    case ItemType::ItemTypeVideo: {
        drawImgAndVideo(painter, option, index);
        break;
    }
    default:
        drawImgAndVideo(painter, option, index);
        break;
    }
    return;
}

void ThumbnailDelegate::drawImgAndVideo(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    const DBImgInfo data = itemData(index);
    bool selected = false;
    if ((option.state & QStyle::State_Selected) != 0) {
        selected = true;
    }
    painter->setRenderHints(QPainter::HighQualityAntialiasing |
                            QPainter::SmoothPixmapTransform |
                            QPainter::Antialiasing);
    QRect backgroundRect = option.rect;
    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    //选中阴影框
    if (selected) {
        QPainterPath backgroundBp;
        backgroundBp.addRoundedRect(backgroundRect, utils::common::SHADOW_BORDER_RADIUS, utils::common::SHADOW_BORDER_RADIUS);
        painter->setClipPath(backgroundBp);

        QBrush shadowbrush;

        if (themeType == DGuiApplicationHelper::LightType) {
            shadowbrush = QBrush(QColor("#DEDEDE"));
        }
        if (themeType == DGuiApplicationHelper::DarkType) {
            shadowbrush = QBrush(QColor("#4F4F4F"));
        }
        painter->fillRect(backgroundRect, shadowbrush);

        //绘制选中默认背景 屏蔽了好像也没影响
        //QRect backRect(backgroundRect.x() + 8, backgroundRect.y() + 8, backgroundRect.width() - 16, backgroundRect.height() - 16);
        //QPainterPath backBp;
        //backBp.addRoundedRect(backRect, utils::common::BORDER_RADIUS, utils::common::BORDER_RADIUS);
        //painter->setClipPath(backBp);
        //painter->fillRect(backRect, shadowbrush);
    }

    QRect pixmapRect;
    QImage img = ImageDataService::instance()->getThumnailImageByPathRealTime(data.filePath, COMMON_STR_TRASH == m_imageTypeStr);
    if (img.isNull()) {
        pixmapRect.setX(backgroundRect.x() + backgroundRect.width() / 2 - NotSupportedOrDamagedWidth / 2);
        pixmapRect.setY(backgroundRect.y() + backgroundRect.height() / 2 - NotSupportedOrDamagedHeigh / 2);
        pixmapRect.setWidth(NotSupportedOrDamagedWidth);
        pixmapRect.setHeight(NotSupportedOrDamagedHeigh);
    } else {
        pixmapRect.setX(backgroundRect.x() + 8);
        pixmapRect.setY(backgroundRect.y() + 8);
        pixmapRect.setWidth(backgroundRect.width() - 16);
        pixmapRect.setHeight(backgroundRect.height() - 16);
    }
    //2020/6/9 DJH UI 透明图片背景
    QBrush transparentbrush;
    if (themeType == DGuiApplicationHelper::LightType) {
        transparentbrush = QBrush(QColor("#FFFFFF"));
    } else if (themeType == DGuiApplicationHelper::DarkType) { //#BUG77517，去除下方的虚化代码，改为直接填充黑色
        transparentbrush = QBrush(QColor("#000000"));
    }
    QRect transparentRect(backgroundRect.x() + 8, backgroundRect.y() + 8, backgroundRect.width() - 16, backgroundRect.height() - 16);
    QPainterPath transparentBp;
    transparentBp.addRoundedRect(transparentRect, utils::common::BORDER_RADIUS, utils::common::BORDER_RADIUS);
    painter->setClipPath(transparentBp);
    painter->fillRect(transparentRect, transparentbrush);

    QPainterPath bp1;
    bp1.addRoundedRect(pixmapRect, utils::common::BORDER_RADIUS, utils::common::BORDER_RADIUS);
    painter->setClipPath(bp1);

    if (!ImageDataService::instance()->imageIsLoaded(data.filePath, COMMON_STR_TRASH == m_imageTypeStr)) {
        painter->drawPixmap(pixmapRect, m_default);
    } else {
        if (img.isNull()) {
            if (data.itemType == ItemTypeVideo) {
                painter->drawPixmap(pixmapRect, m_videoDefault);
            } else {
                painter->drawPixmap(pixmapRect, m_damagePixmap);
            }
        } else {
            painter->drawPixmap(pixmapRect, QPixmap::fromImage(img));
        }
    }

    //绘制选中图标
    if (selected) {
        QPixmap selectedPixmap;
        if (themeType == DGuiApplicationHelper::LightType) {
            selectedPixmap = selectedPixmapLight;
        } else if (themeType == DGuiApplicationHelper::DarkType) {
            selectedPixmap = selectedPixmapDark;
        }
        QRect selectedRect(backgroundRect.x() + backgroundRect.width() - 30, backgroundRect.y() + 4, 28, 28);
        QPainterPath selectedBp;
        selectedBp.addRoundedRect(selectedRect, utils::common::BORDER_RADIUS, utils::common::BORDER_RADIUS);
        painter->setClipPath(selectedBp);
        painter->drawPixmap(selectedRect, selectedPixmap);
    }

    //绘制剩余天数
    if (COMMON_STR_TRASH == m_imageTypeStr) {
        QPainterPath bp;
        bp.addRoundedRect(backgroundRect, utils::common::BORDER_RADIUS, utils::common::BORDER_RADIUS);
        painter->setClipPath(bp);
        painter->setPen(QColor(85, 85, 85, 170)); //边框颜色：灰色
        //字符串的像素宽度
        QString str(QString::number(data.remainDays) + ThumbnailListView::tr("days"));
        const int m_Width = painter->fontMetrics().width(str);
        painter->setBrush(QBrush(QColor(85, 85, 85, 170)));//填充颜色：灰色
        QFontMetrics Text(str);

        //2020/3/13-xiaolong
        int textwidth = m_Width + 6;        //阴影图框：6：左3像素+右3像素
        int textheight = DFontSizeManager::instance()->fontPixelSize(painter->font());
        int rectwidth = backgroundRect.width() - 8; //缩略图宽度：总宽度减去选中框宽度8
        if (textwidth > rectwidth) { //容纳文字像素的宽度大于缩略图宽度
            textwidth = rectwidth - 4;//减少距离：左右各2
        }
        int tempcha = (rectwidth - textwidth > 4) ? (rectwidth - textwidth - 4) : 4;
        int posx = backgroundRect.x() + tempcha;    //剩余天数起始坐标
        //文字背景圆角矩形框弧度，与字号相关
        int radious = 6;
        if (textheight < 14) {
            radious = 4;
        }

        painter->drawRoundedRect(posx, backgroundRect.y() + backgroundRect.height() - textheight - 14,
                                 textwidth, textheight + 2, radious, radious); //Y参数：backgroundRect宽度-文字宽度-14边距

        painter->setPen(QColor(255, 255, 255));
        if (m_Width - textwidth > 0) {
            str = Text.elidedText(str, Qt::ElideRight, textwidth);
        }
        painter->drawText(posx + 3, backgroundRect.y() + backgroundRect.height() - 15, str);//在框中绘制文字，起点位置离最下方15像素
    }
    //绘制视频时间
    if (data.itemType == ItemTypeVideo) {
        QString str(ImageDataService::instance()->getMovieDurationStrByPath(data.filePath));
        if (str == "-") {
            str = "00:00:00";
        }
        //TODO 暂时屏蔽最近删除界面的视频时间显示，待需求明确显示位置后修改并解除屏蔽
        if (dApp->signalM->getSliderValue() > 2 && !str.isEmpty() && m_delegatetype != AlbumViewTrashType) {
            QPainterPath bp;
            bp.addRoundedRect(backgroundRect, utils::common::BORDER_RADIUS, utils::common::BORDER_RADIUS);
            painter->setClipPath(bp);
            painter->setPen(QColor(85, 85, 85, 170)); //边框颜色：灰色
            //字符串的像素宽度
            painter->setBrush(QBrush(QColor(85, 85, 85, 170)));//填充颜色：灰色

            const int m_Width = painter->fontMetrics().width(str);
            QFontMetrics Text(str);

            //2020/3/13-xiaolong
            int textwidth = m_Width + 6;        //阴影图框：6：左3像素+右3像素
            int textheight = DFontSizeManager::instance()->fontPixelSize(painter->font());
            int rectwidth = backgroundRect.width() - 8; //缩略图宽度：总宽度减去选中框宽度8
            if (textwidth > rectwidth) { //容纳文字像素的宽度大于缩略图宽度
                textwidth = rectwidth - 4;//减少距离：左右各2
            }
            int posx = backgroundRect.x() + backgroundRect.width() - m_Width - 16;  //时长起始坐标
            //文字背景圆角矩形框弧度，与字号相关
            int radious = 6;
            if (textheight < 14) {
                radious = 4;
            }

            painter->drawRoundedRect(posx, backgroundRect.y() + backgroundRect.height() - textheight - 14,
                                     textwidth, textheight + 2, radious, radious); //Y参数：backgroundRect宽度-文字宽度-14边距

            painter->setPen(QColor(255, 255, 255));
            if (m_Width - textwidth > 0) {
                str = Text.elidedText(str, Qt::ElideRight, textwidth);
            }
            painter->drawText(posx + 3, backgroundRect.y() + backgroundRect.height() - 15, str);
        }
    }

    //绘制心形图标
    if (COMMON_STR_FAVORITES == m_imageTypeStr) {
        QPainterPath bp;
        bp.addRoundedRect(backgroundRect, utils::common::BORDER_RADIUS, utils::common::BORDER_RADIUS);
        painter->setClipPath(bp);

        QPixmap favPixmap = utils::base::renderSVG(":/resources/images/other/fav_icon .svg", QSize(20, 20));
        painter->drawPixmap(backgroundRect.x() + 20, backgroundRect.y() + backgroundRect.height() - 30, favPixmap);
    }

    painter->restore();
}

void ThumbnailDelegate::onThemeTypeChanged(int themeType)
{
    Q_UNUSED(themeType)
    m_damagePixmap = utils::image::getDamagePixmap(DApplicationHelper::instance()->themeType() == DApplicationHelper::LightType);

    if (DApplicationHelper::instance()->themeType() == DApplicationHelper::LightType) {
        m_default = utils::base::renderSVG(":/icons/deepin/builtin/icons/light/picture_default_light.svg", QSize(60, 45));
        m_videoDefault = utils::base::renderSVG(":/icons/deepin/builtin/icons/light/video_default_light.svg", QSize(60, 45));
    } else {
        m_default = utils::base::renderSVG(":/icons/deepin/builtin/icons/dark/picture_default_dark.svg", QSize(60, 45));
        m_videoDefault = utils::base::renderSVG(":/icons/deepin/builtin/icons/dark/video_default_dark.svg", QSize(60, 45));
    }
}

QSize ThumbnailDelegate::sizeHint(const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const
{
    Q_UNUSED(option)
    DBImgInfo data = index.data(Qt::DisplayRole).value<DBImgInfo>();
    if (data.itemType == ItemType::ItemTypeBlank
            || data.itemType == ItemType::ItemTypeTimeLineTitle
            || data.itemType == ItemType::ItemTypeImportTimeLineTitle) {
        QSize size(data.imgWidth, data.imgHeight);
        return size;
    } else {
        return m_size;
    }
}

DBImgInfo ThumbnailDelegate::itemData(const QModelIndex &index) const
{
    DBImgInfo data = index.data(Qt::DisplayRole).value<DBImgInfo>();
    data.isSelected = index.data(Qt::UserRole).toBool();
    return data;
}

bool ThumbnailDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    Q_UNUSED(model);
    if (!index.isValid())
        return false;
    //跟随收藏按钮变化位置
    QRect rect = QRect(option.rect.x() + 20, option.rect.y() + option.rect.height() - 30, 20, 20);
    QMouseEvent *pMouseEvent = static_cast<QMouseEvent *>(event);
    if (COMMON_STR_FAVORITES == m_imageTypeStr) {
        if (event->type() == QEvent::MouseButtonPress) {
            const DBImgInfo data = itemData(index);
            if (rect.contains(pMouseEvent->pos())) {
                emit sigCancelFavorite(index);
            } else if (event->type() == QEvent::MouseButtonPress && rect.contains(pMouseEvent->x(), pMouseEvent->y() + 27)) {
                emit sigCancelFavorite(index);
            }
        }
    }
    return false;
}
