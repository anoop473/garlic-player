#include "files_controller.h"

FilesController::FilesController(QObject *parent) : BaseController{parent}
{

}

void FilesController::setInventoryTable(DB::InventoryTable *it)
{
    MyInventoryTable = it;
}

QString FilesController::responseFind(int max_results, int begin)
{
    QList<DB::InventoryDataset> results;

    if (max_results > 0)
        results = MyInventoryTable->findPaginated(max_results, begin);
    else
       results = MyInventoryTable->findAll();

    createJsonFromList(results);
    JsonResponse.insertIntegerValuePair("nextPageToken", (max_results + begin));

    return JsonResponse.asString(false).toUtf8();
}

void FilesController::createJsonFromList(QList<DB::InventoryDataset> results)
{
    QJsonArray json_array;
    for (QList<DB::InventoryDataset>::iterator i = results.begin(); i != results.end(); i++)
    {
        json_array.append(createObject(*i));
    }
    JsonResponse.insertArrayValuePair("items", json_array);

}

QJsonObject FilesController::createObject(DB::InventoryDataset dataset)
{
    QJsonObject json;
    json.insert("id", getObjectId(dataset.cache_name));
    json.insert("etag", "");
    json.insert("mimeType", dataset.content_type);
    json.insert("createdDate", "");
    json.insert("modifiedDate", dataset.last_update.toString(Qt::ISODate));
    json.insert("downloadPath", dataset.cache_name);
    json.insert("fileSize", QString::number(dataset.content_length));
    json.insert("transferredSize", QString::number(determineTransferLength(dataset.cache_name, dataset.state)));
    json.insert("completed", (dataset.state == DB::COMPLETE));
    return json;
}

QString FilesController::getObjectId(QString file_name)
{
    QFileInfo fi(file_name);
    return fi.baseName();
}

QString FilesController::getState(int state)
{
    switch (state)
    {
        case DB::TRANSFER:
            return "COMPLETE";
        case DB::STREAMING:
            return "STREAMING";
        case DB::CHECKSUM:
            return "CHECKSUM";
        case DB::CHECKSUM_ERROR:
            return "CHECKSUM_ERROR";
        case DB::COMPLETE:
            return "COMPLETE";
        default:
            return "ERROR";
     }
}
qint64 FilesController::determineTransferLength(QString file_name, int state)
{
    QFileInfo fi(MyConfiguration->getPaths("cache") + file_name);
    switch (state)
    {
        case DB::COMPLETE:
            return fi.size();
        case DB::TRANSFER:
             fi.setFile(fi.absoluteFilePath());
             if (fi.exists())
                 return fi.size();
             else
                 return 0;
        default:
            return 0;

    }

}
