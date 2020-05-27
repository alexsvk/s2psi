select count(project_id),project_name from visit join project on visit_project = project_id join
			visit_subject on visit_id = visit_subject_visit_id join
			subject on visit_subject_subject_id = subject_id
where project_id  >=  7 or project_id != 8;
group by project_name having 1;
select count(project_id),project_name from visit join project on visit_project = project_id join
			visit_subject on visit_id = visit_subject_visit_id join
			subject on visit_subject_subject_id = subject_id
where project_id  >=  1;
group by project_name;	
select project_id,project_name, visit_name from visit join project on visit_project = project_id join
			visit_subject on visit_id = visit_subject_visit_id join
			subject on visit_subject_subject_id = subject_id
where project_id  >=  1;

select all count(project_id), project_name from
                                (
                                (
                                        visit join project on visit_project = project_id
        )
        join
        visit_subject on visit_id = visit_subject_visit_id
                                                )
                                        join
                                subject on visit_subject_subject_id = subject_id
 where project_id >= 7 and project_id != 8 group by project_name having 1;
select all project_id from
           (
                        (
                                visit join project on visit_project = project_id

)
join
visit_subject on visit_id = visit_subject_visit_id
                                        )
                                join
                        subject on visit_subject_subject_id = subject_id ;
			
select all project_id, project_name from
                                (
                                (
                                        visit join project on visit_project = project_id
        )
        join
        visit_subject on visit_id = visit_subject_visit_id
                                                )
                                        join
                                subject on visit_subject_subject_id = subject_id
 where project_id >= 7 and project_id != 8;
 
select all * from
                                                (
                                                (
                                                        visit join project on visit_project = project_id
                       )
                        join
                        visit_subject on visit_id = visit_subject_visit_id
                                                                )
                                                        join
                                                subject on visit_subject_subject_id = subject_id where 1 order by visit_subject_subject_id desc;
						
insert into basic(basic_test_int,basic_test_varchar,basic_test_bool) values(1,"CZK",1);

select count(project_id),subject_height as height 
from visit join project on visit_project = project_id join
visit_subject on visit_id = visit_subject_visit_id join
subject on visit_subject_subject_id = subject_id
where subject_sex != 2 and subject_sex != 3
group by height;

select subject_height as height, subject_sex as sex, (subject_weight) as w
from visit join project on visit_project = project_id join
visit_subject on visit_id = visit_subject_visit_id join
subject on visit_subject_subject_id = subject_id
limit 5,5;
create table temp (c int);
select module_id from module where module_name = 'idaiicebci';