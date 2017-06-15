#pragma once

#include <ctime>

#include <mysql.h>
#pragma comment(lib, "libmysql.lib")
#define DB_HOST "127.0.0.1"
#define DB_USER "swproject"
#define DB_PASS "y0urp@ssword"
#define DB_PORT 3306

///////////////////////////////////////////////////////////////////////////////////////////////////
// MYSQL
///////////////////////////////////////////////////////////////////////////////////////////////////

// ���Ǹ� ���� ���������� ��
static MYSQL mysql_con;

void mysql_init_connection() {
	mysql_init(&mysql_con);
	if (!mysql_real_connect(&mysql_con, DB_HOST, DB_USER, DB_PASS, NULL, DB_PORT, (char *)NULL, 0)) {
		perror(mysql_error(&mysql_con));
		exit(1);
	}
	printf("Success to connect to db\n");
}

string getExtension(string pathName) {
	return pathName.substr(pathName.find_last_of(".") + 1);
}

int mysql_insert_image(const char *hash, const char *realpath, const char *ip = NULL) {
	char sqlQueryStatement[1024] = {};
	string ext = getExtension(realpath);
	printf("realpath: [%s]\n", realpath);
	printf("Extension: [%s]\n", ext.c_str());
	sprintf(
		sqlQueryStatement,
		"INSERT INTO imagehouse.images(`name`, `extension`, `realpath`, `ip`) VALUES (\"%s\", UPPER(\"%s\"), \"%s\", \"%s\")",
		hash,
		ext.c_str() ? ext.c_str() : "",
		realpath ? realpath : "error",
		ip ? ip : "0.0.0.0"
	);

	int queryStat = -1;
	try {
		string safetyStatement;
		for (int i = 0; sqlQueryStatement[i]; ++i) {
			char c = sqlQueryStatement[i];
			if (c == '\\') safetyStatement += "\\\\";
			else safetyStatement.push_back(c);
		}
		queryStat = mysql_query(&mysql_con, safetyStatement.c_str());
	}
	catch (int e) {
		// UniqueŰ�� ���ļ� ������ ������ ���ɼ��� ����.
		if (e != 0) printf("\nINSERT ERROR!\n");
	}

	if (queryStat == 0){
		printf("\nINSERT SUCCESS!\n");
	}
	else {
		printf("\nFAILED QUERY");
	}

	return queryStat;
}

void mysql_close_connection() {
	mysql_close(&mysql_con);
}