/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
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
#ifndef TTBCONTENT_H
#define TTBCONTENT_H

#include <QWidget>
#include <QLabel>
#include "controller/viewerthememanager.h"
//#include <dlistwidget.h>
//#include <DListWidget>
#include <DListWidget>
#include <DSpinner>
//#include <DtkWidgets>
//#include "dlistwidget.h"
#include <QListWidget>
#include <DListView>
#include <QAbstractItemModel>
#include <QStandardItem>
#include "dbmanager/dbmanager.h"
#include <DAnchors>
#include <dimagebutton.h>
#include <DThumbnailProvider>
#include <QPropertyAnimation>
#include <QHBoxLayout>
#include <DIconButton>
#include <DBlurEffectWidget>
#include <DGuiApplicationHelper>
#include <DLabel>

DWIDGET_USE_NAMESPACE


class PushButton;
class ReturnButton;
class ElidedLabel;
class QAbstractItemModel;
//class DImageButton;
class ImageButton;
class ImageItem : public DLabel{
    Q_OBJECT
public:
    ImageItem(int index= 0,QString path = NULL, QString imageType = NULL, QWidget *parent = 0);
    void setIndexNow(int i);
    void setPic(QPixmap pixmap);

    QString _path = NULL;
signals:
    void imageItemclicked(int index,int indexNow);
protected:
    void mousePressEvent(QMouseEvent *ev) override;
    void paintEvent(QPaintEvent *event) override;
private:
    int _index;
    int _indexNow = -1;
    DLabel *_image=nullptr;
    QPixmap _pixmap;
    DSpinner* m_spinner;
    QString m_pixmapstring;
};
class TTBContent : public QLabel
{
    Q_OBJECT
public:
    explicit TTBContent(bool inDB, DBImgInfoList m_infos , QWidget *parent = 0);

signals:
    void ttbcontentClicked();
    void imageClicked(int index,int addIndex);
    void resetTransform(bool fitWindow);
    void rotateClockwise();
    void rotateCounterClockwise();

    void removed();
    void imageEmpty(bool v);
    void contentWidthChanged(int width);
    void showPrevious();
    void showNext();

public slots:
    void setCurrentDir(QString text);
    void setImage(const QString &path,DBImgInfoList infos);
    void updateCollectButton();

private slots:
    void onThemeChanged(ViewerThemeManager::AppTheme theme);
    void updateFilenameLayout();

protected:
    void resizeEvent(QResizeEvent *event);

public:
    QString m_imageType;

private:
#ifndef LITE_DIV
    PushButton *m_folderBtn;
    ReturnButton *m_returnBtn;
#endif
    bool m_inDB;
    bool m_bClBTChecked;

    DIconButton* m_adaptImageBtn;
    DIconButton* m_adaptScreenBtn;
    DIconButton* m_clBT;
    DIconButton* m_rotateLBtn;
    DIconButton* m_rotateRBtn;
    DIconButton* m_trashBtn;
    DIconButton *m_preButton;
    DIconButton *m_nextButton;
    DIconButton *m_backButton;
    ElidedLabel* m_fileNameLabel;
    DWidget *m_imgList;
    QHBoxLayout *m_imglayout;
    DWidget *m_imgListView;
    DWidget *m_preButton_spc;
    DWidget *m_nextButton_spc;
    DBImgInfoList m_imgInfos ;
    QString m_imagePath;
    int m_windowWidth;
    int m_contentWidth;
    int m_lastIndex = 0;
    int m_nowIndex = 0;
    int m_imgInfos_size = 0;
    int m_startAnimation = 0;
};

#endif // TTLCONTENT_H
