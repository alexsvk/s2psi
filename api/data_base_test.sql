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
select all count(project_id), project_name from
           (
                        (
                                visit join project on visit_project = project_id

)
join
visit_subject on visit_id = visit_subject_visit_id
                                        )
                                join
                        subject on visit_subject_subject_id = subject_id where project_id >= 7 or project_id != 8 and project_notes in ('dfdbaehgcb');