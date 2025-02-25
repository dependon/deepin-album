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
#include "fileinotify.h"
#include "application.h"
#include "imageengineapi.h"
#include "signalmanager.h"
#include "utils/unionimage.h"
#include "baseutils.h"
#include "imageutils.h"
#include "signalmanager.h"

#include <sys/inotify.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QDir>

enum {MASK = IN_MODIFY | IN_CREATE | IN_DELETE};

FileInotify::FileInotify(QObject *parent)
    : QObject(parent)
    , m_Supported(UnionImage_NameSpace::unionImageSupportFormat() + utils::base::m_videoFiletypes) //图片+视频
{
    for (auto &eachData : m_Supported) {
        eachData = eachData.toUpper();
    }

    m_timer = new QTimer();
    connect(m_timer, &QTimer::timeout, this, &FileInotify::onNeedSendPictures);

    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, [this](const QString & path) {
        Q_UNUSED(path)
        m_timer->start(500);
    });
}

FileInotify::~FileInotify()
{
    clear();
}

void FileInotify::checkNewPath()
{
    for (const auto &currentDir : m_currentDirs) {
        QDir dir(currentDir);
        QFileInfoList list;
        utils::image::getAllDirInDir(dir, list);

        QStringList dirs;
        std::transform(list.begin(), list.end(), std::back_inserter(dirs), [](const QFileInfo & info) {
            return info.absoluteFilePath();
        });

        if (!dirs.isEmpty()) {
            m_watcher.addPaths(dirs);
        }
    }
}

void FileInotify::addWather(const QStringList &paths, const QString &album, int UID)
{
    m_currentDirs = paths;
    m_currentAlbum = album;
    m_currentUID = UID;
    m_watcher.addPaths(paths);
    m_timer->start(1500);
}

/*void FileInotify::removeWatcher(const QString &path)
{ 需要的话，得重写一下
    QMutexLocker loker(&m_mutex);
    if (!isVaild())
        return;
    auto it = watchedDirId.find(path);
    if (it != watchedDirId.end()) {
        inotify_rm_watch(m_handleId, it.value());
        watchedDirId.erase(it);
    }
}*/

void FileInotify::clear()
{
    m_Supported.clear();
    m_newFile.clear();
    m_Supported.clear();
    if (m_timer->isActive()) {
        m_timer->stop();
        delete m_timer;
        m_timer = nullptr;
    }
}

void FileInotify::getAllPicture(bool isFirst)
{
    QFileInfoList list;
    for (int i = 0; i != m_currentDirs.size(); ++i) {
        QDir dir(m_currentDirs[i]);
        if (!dir.exists()) {
            m_currentDirs.removeAt(i);
            --i;
            continue;
        }

        //获取监控目录的所有文件
        utils::image::getAllFileInDir(dir, list);
    }

    if (m_currentDirs.isEmpty()) { //文件夹被删除，清理数据库
        DBManager::instance()->removeCustomAutoImportPath(m_currentUID);
        m_deleteFile = DBManager::instance()->getPathsByAlbum(m_currentUID);
        m_newFile.clear();
        emit pathDestroyed(m_currentUID);
        return;
    }

    //移除不支持的文件
    auto removeIter = std::remove_if(list.begin(), list.end(), [this](const QFileInfo & info) {
        return !m_Supported.contains(info.suffix().toUpper());
    });
    list.erase(removeIter, list.end());

    //提取文件路径
    QStringList filePaths;
    std::transform(list.begin(), list.end(), std::back_inserter(filePaths), [](const QFileInfo & info) {
        return info.isSymLink() ? info.readLink() : info.absoluteFilePath();
    });

    //获取当前已导入的全部文件
    auto allPaths = DBManager::instance()->getPathsByAlbum(m_currentUID);

    //筛选出新增图片文件
    for (auto path : filePaths) {
        if (!allPaths.contains(path)) {
            m_newFile << path;
        }
    }

    //筛选出删除图片文件，初次导入不需要执行
    if (!isFirst) {
        for (auto path : allPaths) {
            if (!filePaths.contains(path)) {
                m_deleteFile << path;
            }
        }
    }
}

//void FileInotify::fileNumChange()
//{
//    QDir dir(m_currentDir);
//    if (!dir.exists()) {
//        return;
//    }
//    dir.setFilter(QDir::Files | QDir::NoSymLinks); //设置类型过滤器，只为文件格式
//    dir.setNameFilters(m_Supported);
//    unsigned int dir_count = dir.count();
//    if (dir_count <= 0) {
//        return;
//    }
//    QFileInfoList list = dir.entryInfoList();
//    for (auto fileinfo : list) {
//        if (m_Supported.contains(fileinfo.suffix().toUpper())) {
//            QString filename = fileinfo.fileName();
//            filename += m_currentDir;
//            if (!m_allPic.contains(filename)) {
//                qDebug() << "filename add: " << filename;
//                m_allPic << filename;
//            }
//        }
//    }
//}

void FileInotify::onNeedSendPictures()
{
    checkNewPath();
    getAllPicture(false);

    //发送导入
    if (!m_newFile.isEmpty() || !m_deleteFile.isEmpty()) {
        emit dApp->signalM->sigMonitorChanged(m_newFile, m_deleteFile, m_currentAlbum, m_currentUID);
        if (m_newFile.size() > 100) {
            QStringList().swap(m_newFile); //强制清理内存
        } else {
            m_newFile.clear();
        }

        if (m_deleteFile.size() > 100) {
            QStringList().swap(m_deleteFile); //强制清理内存
        } else {
            m_deleteFile.clear();
        }
    }

    m_timer->stop();
}
