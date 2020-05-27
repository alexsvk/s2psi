CREATE TABLE country
(
	'country@id' INTEGER,
	'country@name' TEXT,
	'country@printable'_name TEXT,
	'country@iso2' TEXT,
	'country@iso3' TEXT,
	'country@numcode' INTEGER,
	CONSTRAINT PK_country PRIMARY KEY ('country@id')
);
CREATE TABLE device
(
	'device@id' INTEGER,
	'device@name' TEXT,
	CONSTRAINT PK_Device PRIMARY KEY ('device@id')
);
CREATE TABLE test_set
(
	'test_set@id' INTEGER,
	'test_set@name' TEXT,
	CONSTRAINT PK_test_set PRIMARY KEY ('test_set@id')
);
CREATE TABLE device_test_set
(
	'device_test_set@device_id' INTEGER,
	'device_test_set@test_set_id' INTEGER,
	CONSTRAINT PK_device_test_set PRIMARY KEY ('device_test_set@device_id', 'device_test_set@test_set_id'),
	CONSTRAINT FK_device_test_set_device FOREIGN KEY ('device_test_set@device_id')
		REFERENCES device('device@id') ON DELETE RESTRICT ON UPDATE CASCADE,
	CONSTRAINT FK_device_test_set_test_set FOREIGN KEY ('device_test_set@test_set_id')
		REFERENCES test_set('test_set@id') ON DELETE RESTRICT ON UPDATE CASCADE
);

CREATE INDEX device_test_set_device_id_idx ON device_test_set('device_test_set@device_id');
CREATE INDEX device_test_set_test_set_id_idx ON device_test_set('device_test_set@test_set_id');
CREATE TABLE dominant_arm
(
	'dominant_arm@id' INTEGER,
	'dominant_arm@name' TEXT,
	CONSTRAINT PK_dominant_arm PRIMARY KEY ('dominant_arm@id')
);
CREATE TABLE dominant_leg
(
	'dominant_leg@id' INTEGER,
	'dominant_leg@name' TEXT,
	CONSTRAINT PK_dominant_leg PRIMARY KEY ('dominant_leg@id')
);
CREATE TABLE module
(
	'module@id' INTEGER,
	'module@name' TEXT,
	CONSTRAINT PK_module PRIMARY KEY ('module@id')
);
CREATE TABLE MIP_group 
( 
	'MIP_group@module_group_id' INTEGER NOT NULL,
	'MIP_group@name' TEXT NOT NULL,
	'MIP_group@module' INTEGER NOT NULL,
	'MIP_group@column_name' TEXT NOT NULL,    --  Name of the column in the measurement table in the database. 
	'MIP_group@test_set' INTEGER NOT NULL, -- primary key integer (used as rowid automatically)
	CONSTRAINT PK_MIP_group PRIMARY KEY ('MIP_group@module_group_id'),
	CONSTRAINT UQ_MIP_group_column_name UNIQUE ('MIP_group@column_name'),
	CONSTRAINT FK_MIP_group_test_set FOREIGN KEY ('MIP_group@test_set')  REFERENCES test_set,
	CONSTRAINT FK_MIP_group_module FOREIGN KEY ('MIP_group@module') REFERENCES 'module'
);
CREATE INDEX MIP_group_test_set_idx ON MIP_group('MIP_group@test_set');
CREATE INDEX MIP_group_module_idx ON MIP_group('MIP_group@module');
CREATE TABLE MIP_parameter
(
	'MIP_parameter@id' INTEGER,
	'MIP_parameter@name' INTEGER,
	'MIP_parameter@MIP_group' INTEGER,
	'MIP_parameter@editable' INTEGER,
	CONSTRAINT PK_MIP_parameter PRIMARY KEY ('MIP_parameter@id'),
	CONSTRAINT FK_MIP_parameter_MIP_group FOREIGN KEY ('MIP_parameter@MIP_group')
		REFERENCES MIP_group('MIP_group@module_group_id') ON DELETE CASCADE ON UPDATE CASCADE
);
CREATE INDEX MIP_parameter_MIP_group_idx ON MIP_parameter('MIP_parameter@MIP_group');
CREATE TABLE MRP_parameter
(
	'MRP_parameter@id' INTEGER,
	'MRP_parameter@name' TEXT,
	'MRP_parameter@description' TEXT,
	'MRP_parameter@abbreviation' TEXT,
	'MRP_parameter@unit' TEXT,
	'MRP_parameter@basic' INTEGER,
	'MRP_parameter@editable' INTEGER,
	'MRP_parameter@MRP_group' INTEGER,
	'MRP_parameter@column_name' TEXT,
	CONSTRAINT PK_MRP_parameter PRIMARY KEY ('MRP_parameter@id'),
	CONSTRAINT FK_MRP_parameter_MRP_group FOREIGN KEY ('MRP_parameter@MRP_group')
		REFERENCES MRP_group ON DELETE CASCADE ON UPDATE CASCADE
);
CREATE INDEX MRP_parameter_MRP_group_idx ON MRP_parameter('MRP_parameter@MRP_group');
CREATE TABLE project
(
	'project@id' INTEGER,
	'project@name' TEXT,
	'project@notes' TEXT,
	CONSTRAINT PK_project PRIMARY KEY ('project@id')
);
CREATE TABLE project_module
(
	'project_module@project_id' INTEGER,
	'project_module@id' INTEGER,
	CONSTRAINT PK_project_module PRIMARY KEY ('project_module@project_id', 'project_module@id'),
	CONSTRAINT FK_project_module_module_id FOREIGN KEY ('project_module@id')
		REFERENCES module ON DELETE CASCADE ON UPDATE CASCADE,
	CONSTRAINT FK_project_module_project_id FOREIGN KEY ('project_module@project_id')
		REFERENCES project ON DELETE CASCADE ON UPDATE CASCADE
);
CREATE INDEX project_module_project_id_idx ON project_module('project_module@project_id');
CREATE TABLE sex
(
	'sex@id' INTEGER,
	'sex@name' TEXT,
	CONSTRAINT PK_sex PRIMARY KEY ('sex@id')
);
CREATE TABLE subject
(
	'subject@id' INTEGER,
	'subject@first_name' TEXT,
	'subject@middle_name' TEXT,
	'subject@last_name' TEXT,
	'subject@date_of_birth' TEXT,
	'subject@sex' INTEGER,
	'subject@address' TEXT,
	'subject@postal_code' TEXT,
	'subject@city' TEXT,
	'subject@country' INTEGER,
	'subject@phone' TEXT,
	'subject@email' TEXT,
	'subject@height' REAL,
	'subject@weight' REAL,
	'subject@foot_size' REAL,
	'subject@picture' TEXT,
	'subject@dominant_leg' INTEGER,
	'subject@dominant_arm' INTEGER,
	'subject@type' INTEGER,
	'subject@type_l1' INTEGER,
	'subject@type_l2' INTEGER,
	'subject@notes' TEXT,
	'subject@files' TEXT,
	CONSTRAINT PK_subject PRIMARY KEY ('subject@id'),
	CONSTRAINT FK_subject_country FOREIGN KEY ('subject@country')
		REFERENCES country ON DELETE RESTRICT ON UPDATE CASCADE,
	CONSTRAINT FK_subject_dominant_arm FOREIGN KEY ('subject@dominant_arm')
		REFERENCES dominant_arm ON DELETE RESTRICT ON UPDATE CASCADE,
	CONSTRAINT FK_subject_dominant_leg FOREIGN KEY ('subject@dominant_leg')
		REFERENCES dominant_leg ON DELETE RESTRICT ON UPDATE CASCADE,
	CONSTRAINT FK_subject_sex FOREIGN KEY ('subject@sex')
		REFERENCES sex ON DELETE RESTRICT ON UPDATE RESTRICT,
	CONSTRAINT FK_subject_subject_type FOREIGN KEY ('subject@type')
		REFERENCES subject_type('subject_type@id') ,
	CONSTRAINT FK_subject_subject_type_l1 FOREIGN KEY ('subject@type_l1')
		REFERENCES subject_type_l1 ON DELETE RESTRICT ON UPDATE CASCADE,
	CONSTRAINT FK_subject_subject_type_l2 FOREIGN KEY ('subject@type_l2')
		REFERENCES subject_type_l2 ON DELETE RESTRICT ON UPDATE CASCADE
);
CREATE INDEX subject_country_idx ON subject('subject@country');
CREATE INDEX subject_dominant_arm_idx ON subject('subject@dominant_arm');
CREATE INDEX subject_dominant_leg_idx ON subject('subject@dominant_leg');
CREATE INDEX subject_sex_idx ON subject('subject@sex');
CREATE INDEX subject_type_idx ON subject('subject@type');
CREATE INDEX subject_type_l1_idx ON subject('subject@type_l1');
CREATE INDEX subject_type_l2_idx ON subject('subject@type_l2');
CREATE TABLE subject_type
(
	'subject_type@id' INTEGER,
	'subject_type@name' TEXT,
	CONSTRAINT PK_subject_type PRIMARY KEY ('subject_type@id')
);
CREATE TABLE subject_type_l1
(
	'subject_type_l1@id' INTEGER,
	'subject_type_l1@name' TEXT,
	CONSTRAINT PK_subject_type_l1 PRIMARY KEY ('subject_type_l1@id')
);
CREATE TABLE subject_type_l2
(
	'subject_type_l2@id' INTEGER,
	'subject_type_l2@name' TEXT,
	CONSTRAINT PK_subject_type_l2 PRIMARY KEY ('subject_type_l2@id')
);
CREATE TABLE visit
(
	'visit@id' INTEGER,
	'visit@name' TEXT,
	'visit@notes 'TEXT,
	'visit@project' INTEGER,
	CONSTRAINT PK_visit PRIMARY KEY ('visit@id'),
	CONSTRAINT FK_visit_project FOREIGN KEY ('visit@project')
		REFERENCES project ON DELETE CASCADE ON UPDATE CASCADE
);
CREATE TABLE visit_subject
(
	'visit_subject@visit_id' INTEGER,
	'visit_subject@subject_id' INTEGER,
	CONSTRAINT PK_visit_subject PRIMARY KEY ('visit_subject@visit_id', 'visit_subject@subject_id'),
	CONSTRAINT FK_visit_subject_subject FOREIGN KEY ('visit_subject@subject_id')
		REFERENCES subject ON DELETE CASCADE ON UPDATE CASCADE,
	CONSTRAINT FK_visit_subject_visit FOREIGN KEY ('visit_subject@visit_id')
		REFERENCES visit ON DELETE CASCADE ON UPDATE CASCADE
);
CREATE INDEX visit_subject_subject_id_idx ON visit_subject('visit_subject@subject_id');
CREATE INDEX visit_subject_visit_id_idx ON visit_subject('visit_subject@visit_id');

CREATE TABLE MRP_group
(
	'mrp_group@id' INTEGER,
	'mrp_group@name' TEXT,
	'mrp_group@module' INTEGER,
	CONSTRAINT PK_mrp_group_mrp_group_id PRIMARY KEY ('mrp_group@id'),
	CONSTRAINT FK_mrp_group_mrp_group_module FOREIGN KEY ('mrp_group@module')
		REFERENCES 'module'('module@id') ON DELETE CASCADE ON UPDATE CASCADE
);