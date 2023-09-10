#include "ItemImporter.h"

#include <DBApi.h>
#include <QFutureWatcher>
#ifdef USE_CONCURRENT
#include <QtConcurrent>
#endif

extern DB_functions_t *DBApi_deadbeef;

typedef struct promise_and_pabort_s {
    QPromise<PlayItemIterator> *promise;
    int *pabort;
} promise_and_pabort_t;

int files_callback(DB_playItem_t *it, void *user_data) {
    promise_and_pabort_t *pp = static_cast<promise_and_pabort_t *>(user_data);
    if (it) {
        DB_metaInfo_t *meta = DBApi_deadbeef->pl_meta_for_key(it, ":URI");
        pp->promise->setProgressValueAndText(pp->promise->future().progressValue()+1, QString(meta->value));
    }
    if (pp->promise->isCanceled()) {
        *pp->pabort = 1;
    }
    return 0;
}

void loadFiles(QPromise<PlayItemIterator> &promise, QStringList files) {
    int pabort = 0;
    promise_and_pabort_t pp = {&promise, &pabort};

    promise.setProgressRange(0, files.size());
    promise.setProgressValue(0);
    promise.start();
    ddb_playlist_t *plt = DBApi_deadbeef->plt_alloc("_q_item_importer");
    PlayItemIterator pit = PlayItemIterator(plt);
    promise.addResult(pit);

    DB_playItem_t *it_last = nullptr;
    int count = 0;
    for (QString &file : files) {
        promise.setProgressValueAndText(count++, file);
        if (promise.isCanceled()) {
            break;
        }
        DB_playItem_t *it_next = DBApi_deadbeef->plt_insert_file2(0, plt, it_last, file.toUtf8().constData(), &pabort, files_callback, &pp);
        if (it_next) {
            DBApi_deadbeef->pl_item_ref(it_next);
            it_last = it_next;
        }
    }
    promise.setProgressValue(count);

    DBApi_deadbeef->plt_unref(plt); // risky
    promise.finish();

}

int folders_callback(ddb_insert_file_result_t result, const char *filename, void *user_data) {
    promise_and_pabort_t *pp = static_cast<promise_and_pabort_t *>(user_data);

    switch (result) {
        case DDB_INSERT_FILE_RESULT_SUCCESS:
        default:
            qDebug() << filename;
            pp->promise->setProgressRange(0,pp->promise->future().progressValue()+1);
            pp->promise->setProgressValueAndText(pp->promise->future().progressValue()+1, filename);
    }

    if (pp->promise->isCanceled()) {
        *pp->pabort = 1;
    }
    //DB_metaInfo_t *meta = DBApi_deadbeef->pl_meta_for_key(it, ":URI");
    //promise->setProgressValueAndText(0, QString(meta->value));
    return 0;
}

//int folders_callback(DB_playItem_t *it, void *user_data) {
//    QPromise<PlayItemIterator> *promise = static_cast<QPromise<PlayItemIterator> *>(user_data);

//    DB_metaInfo_t *meta = DBApi_deadbeef->pl_meta_for_key(it, ":URI");
//    promise->setProgressValueAndText(0, QString(meta->value));
//    return 0;
//}

void loadFolders(QPromise<PlayItemIterator> &promise, QStringList folders) {
    int pabort = 0;
    promise_and_pabort_t pp = {&promise, &pabort};

    promise.setProgressRange(0, 1000);
    promise.setProgressValue(0);
    promise.start();
    ddb_playlist_t *plt = DBApi_deadbeef->plt_alloc("_q_item_importer");
    PlayItemIterator pit = PlayItemIterator(plt);
    promise.addResult(pit);

    DB_playItem_t *it_last = nullptr;
    for (QString &folder : folders) {
        promise.setProgressValueAndText(1, folder);
        if (promise.isCanceled()) {
            break;
        }
        static int visibility = 0;
        static uint32_t flags = 0; // TODO ddb_insert_file_flags_t
        DB_playItem_t *it_next = DBApi_deadbeef->plt_insert_dir3(visibility, flags, plt, it_last, folder.toUtf8().constData(), &pabort, folders_callback, &pp);
        if (it_next) {
            DBApi_deadbeef->pl_item_ref(it_next);
            it_last = it_next;
        }
    }
    promise.setProgressValue(1000);

    DBApi_deadbeef->plt_unref(plt); // risky
    promise.finish();

}

void loadPlaylists(QPromise<PlayItemIterator> &promise, QStringList playlists) {
    int pabort = 0;
    promise_and_pabort_t pp = {&promise, &pabort};

    promise.setProgressRange(0, 10000);
    promise.setProgressValue(0);
    promise.start();
    ddb_playlist_t *plt = DBApi_deadbeef->plt_alloc("_q_item_importer");
    PlayItemIterator pit = PlayItemIterator(plt);
    promise.addResult(pit);

    DB_playItem_t *it_last = nullptr;
    int count = 0;
    for (QString &playlist : playlists) {
        promise.setProgressValueAndText(count++, playlist);
        if (promise.isCanceled()) {
            break;
        }
        DB_playItem_t *it_next = DBApi_deadbeef->plt_load2(0, plt, it_last, playlist.toUtf8().constData(), &pabort, files_callback, &pp);
        if (it_next) {
            DBApi_deadbeef->pl_item_ref(it_next);
            it_last = it_next;
        }
    }
    promise.setProgressValue(10000);

    DBApi_deadbeef->plt_unref(plt); // risky
    promise.finish();
}

QFuture<PlayItemIterator> ItemImporter::runFileImport(QStringList files) {
#if USE_CONCURRENT
    return QtConcurrent::run(loadFiles, files);
#else
    return {};
#endif
}

QFuture<PlayItemIterator> ItemImporter::runFolderImport(QStringList folders) {
#if USE_CONCURRENT
    return QtConcurrent::run(loadFolders, folders);
#else
    return {};
#endif
}

QFuture<PlayItemIterator> ItemImporter::runPlaylistImport(QStringList playlist_files) {
#if USE_CONCURRENT
    return QtConcurrent::run(loadPlaylists, playlist_files);
#else
    return {};
#endif
}
