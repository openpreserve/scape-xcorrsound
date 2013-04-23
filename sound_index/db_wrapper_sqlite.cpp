#include <cstdio>
#include <iostream>
#include <utility>
#include <sstream>
#include <string>
#include <vector>

#include "stdint.h"

#include "db_wrapper.h"
#include "FingerprintInfo.h"

#include "sqlite/sqlite3.h"

namespace {
    sqlite3 *db;
}

std::string db_wrapper::getFilename(size_t id) {
    sqlite3_stmt *ppStmt;
    char sql[256] = "\0";
    sprintf(sql, "select filename from files where fileId = %ld", id);
    
    if (SQLITE_OK != sqlite3_prepare_v2(db, sql, -1, &ppStmt, 0)) {
		std::cout << "Well here's an error..." << std::endl;
		//error handling
    }

	if (sqlite3_step(ppStmt) != SQLITE_DONE) {
		std::string filename = ((char*)sqlite3_column_text(ppStmt, 0));
		sqlite3_finalize(ppStmt);
		return filename;
	} else {
		sqlite3_finalize(ppStmt);
		return std::string("");
	}
}

db_wrapper::db_wrapper(char dbFile[]) {

    char *errMsg = 0;
    int returnCode = sqlite3_open(dbFile, &db);
    
    if (returnCode != SQLITE_OK) {
	std::cout << "Database connection unsuccesful" << std::endl;
    }

}

db_wrapper::~db_wrapper() {
    sqlite3_close(db);
}

int db_wrapper::insert(size_t fingerprint, FingerprintInfo &info) {
    sqlite3_stmt *ppStmt;
    std::stringstream ss;
    ss << "INSERT INTO fingerprints (fingerprint,fileId,offset) VALUES ('";
    ss << fingerprint << "','" << info.fileId << "','" << info.offset << "');";
    const char * sql = ss.str().c_str();
    int rc;
    if (SQLITE_OK != (rc = sqlite3_prepare_v2(db, sql, -1, &ppStmt, 0))) {
	// error handling
	std::cout << "something went wrong. Error code: " << rc << std::endl;
	std::cout << ss.str() << std::endl;
	std::cout << sqlite3_errmsg(db) << std::endl;
    }

    if (SQLITE_DONE != (rc = sqlite3_step(ppStmt))) {
	std::cout << "something else went wrong. Error code: " << rc << std::endl;
	std::cout << sqlite3_errmsg(db) << std::endl;
	// error handling
    }

    sqlite3_finalize(ppStmt);

    return 0;
}

int db_wrapper::bulk_insert(std::vector<std::pair<size_t, FingerprintInfo> > &data) {
    sqlite3_stmt *ppStmt;

    char sql[256] = "\0";
    sprintf(sql, "INSERT INTO fingerprints VALUES (?, ?, ?)");
    sqlite3_prepare_v2(db, sql, 256, &ppStmt, NULL);

    char *errmsg;
    sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &errmsg);

    for (size_t i = 0; i < data.size(); ++i) {

	sqlite3_bind_int(ppStmt, 1, data[i].first);
	sqlite3_bind_int(ppStmt, 2, data[i].second.offset);
	sqlite3_bind_int(ppStmt, 3, data[i].second.fileId);

	int rc =  sqlite3_step(ppStmt);
	
	if (rc != SQLITE_DONE) {
	    std::cout << sqlite3_errmsg(db) << std::endl;
	    std::cout << data[i].second.fileId << std::endl;
	}

	sqlite3_clear_bindings(ppStmt);
	sqlite3_reset(ppStmt);

    }

    sqlite3_finalize(ppStmt);

    sqlite3_exec(db, "END TRANSACTION", NULL, NULL, &errmsg);

    // ss << "INSERT INTO fingerprints (fingerprint,fileId,offset) VALUES ";
    // for (size_t i = 0; i < data.size(); ++i) {
    // 	if (i > 0) ss << ",";
    // 	ss << "('" << data[i].first << "','" << data[i].second.fileId;
    // 	ss << "','" << data[i].second.offset << "')";
    // }

    // const char * sql = ss.str().c_str();
    // int rc;
    // if (SQLITE_OK != (rc = sqlite3_prepare_v2(db, sql, -1, &ppStmt, 0))) {
    // 	// error handling
    // 	std::cout << "something went wrong. Error code: " << rc << std::endl;
    // 	std::cout << ss.str() << std::endl;
    // 	std::cout << sqlite3_errmsg(db) << std::endl;
    // }

    // if (SQLITE_DONE != (rc = sqlite3_step(ppStmt))) {
    // 	std::cout << "something else went wrong. Error code: " << rc << std::endl;
    // 	std::cout << sqlite3_errmsg(db) << std::endl;
    // 	// error handling
    // }

    // sqlite3_finalize(ppStmt);



    return 0;    
}

int db_wrapper::query(size_t fingerprint, std::vector<FingerprintInfo> &result) {
    sqlite3_stmt *ppStmt;
    char sql[256] = "\0";
    sprintf(sql, "select * from fingerprints where fingerprint = %ld", fingerprint);
    
    if (SQLITE_OK != sqlite3_prepare_v2(db, sql, -1, &ppStmt, 0)) {
	std::cout << "Well here's an error..." << std::endl;
	//error handling
    }

    while (sqlite3_step(ppStmt) != SQLITE_DONE) {
	uint32_t fingerprint = sqlite3_column_int(ppStmt, 0);
	int32_t offset = sqlite3_column_int(ppStmt, 1);
	int32_t fileId = sqlite3_column_int(ppStmt, 2);
	FingerprintInfo info(fileId, offset);
	result.push_back(info);
    }

    sqlite3_finalize(ppStmt);

    return 0;
}

/**
 * This method is susceptible to injections. Please make sure filename is 'safe'
 */
uint32_t db_wrapper::insert_file(std::string filename) {
    {
	sqlite3_stmt *ppStmt;
	char sql[1024] = "\0";
	sprintf(sql, "select fileId from files where filename like '%s'", filename.c_str());

	if (SQLITE_OK != sqlite3_prepare_v2(db, sql, -1, &ppStmt, 0)) {
	    std::cout << "Well here's an error..." << std::endl;
	    //error handling
	}

	uint32_t fileId = 0;
	if (sqlite3_step(ppStmt) != SQLITE_DONE) {
	    fileId = sqlite3_column_int(ppStmt,0);
	    sqlite3_finalize(ppStmt);
	    return fileId;
	}
	sqlite3_finalize(ppStmt);
    }

    sqlite3_stmt *ppStmt;
    char sql[1024] = "\0";
    sprintf(sql, "insert into files (filename) VALUES ('%s')", filename.c_str());

    if (SQLITE_OK != sqlite3_prepare_v2(db, sql, -1, &ppStmt, 0)) {
	std::cout << "Well here's an error..." << std::endl;
	//error handling
    }

    uint32_t fileId = 0;
    int rc;
    if (SQLITE_DONE != (rc = sqlite3_step(ppStmt))) {
    	// error handling
    	std::cout << "something went wrong. Error code: " << rc << std::endl;
    	std::cout << sqlite3_errmsg(db) << std::endl;
    }
    fileId = sqlite3_last_insert_rowid(db);

    sqlite3_finalize(ppStmt);

    return fileId;
}
