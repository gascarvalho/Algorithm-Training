#undef NDEBUG
#include <bits/stdc++.h>
#include <boost/container/small_vector.hpp>
#include <range/v3/all.hpp>

template<typename T>
using vec = std::vector<T>;
template<typename K, typename V>
using umap = std::unordered_map<K, V>;
template<typename T>
using uset = std::unordered_set<T>;
using boost::container::small_vector;
using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::pair;
using std::queue;
using std::string;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

static std::random_device RANDOM_DEVICE;
static std::mt19937		  RANDOM_GENERATOR(RANDOM_DEVICE());

using UserID			 = int;
using SkillID			 = int;
using ProjectID			 = int;
using RoleID			 = int;
constexpr int INVALID_ID = -1;

struct User {
	UserID			   id;
	string			   name;
	umap<SkillID, int> skills;

	int level(SkillID skill) const {
		const auto it = skills.find(skill);
		if (it != end(skills))
			return it->second;
		return 0;
	}
};

struct Role {
	RoleID	id;
	SkillID skill;
	int		level;
};

struct Project {
	ProjectID			  id;
	string				  name;
	int					  bb;  // best before date
	int					  score;
	int					  days;	 // how long to complete
	vec<Role>			  roles;
	umap<SkillID, RoleID> skills;  // SkillID -> Role index
};

struct OngoingProject {
	ProjectID				  id;
	vec<pair<UserID, RoleID>> users;
	int						  finishday;

	bool operator<(const OngoingProject& rhs) const {
		return this->finishday > rhs.finishday;
	}
};

static vec<User>			 USERS;
static SkillID				 SKILL_COUNTER = 0;
static umap<SkillID, string> SKILL_ID_TO_NAME;
static umap<string, SkillID> SKILL_NAME_TO_ID;
static vec<Project>			 PROJECTS;

void read_input(std::istream& in) {
	int contributors_n, projects_n;
	in >> contributors_n >> projects_n;

	USERS.reserve(contributors_n);
	for (auto i = 0; i < contributors_n; ++i) {
		string name;
		int	   skills_n;
		in >> name >> skills_n;
		umap<SkillID, int> user_skills;
		const UserID	   user_id = USERS.size();
		for (auto j = 0; j < skills_n; ++j) {
			string skill_name;
			int	   skill_level;
			in >> skill_name >> skill_level;
			if (!SKILL_NAME_TO_ID.contains(skill_name)) {
				const auto skill_id			 = SKILL_COUNTER++;
				SKILL_NAME_TO_ID[skill_name] = skill_id;
				SKILL_ID_TO_NAME[skill_id]	 = skill_name;
			}
			const auto skill_id	  = SKILL_NAME_TO_ID[skill_name];
			user_skills[skill_id] = skill_level;
		}
		USERS.push_back(User{
			.id		= user_id,
			.name	= name,
			.skills = user_skills,
		});
	}

	PROJECTS.reserve(projects_n);
	for (auto i = 0; i < projects_n; ++i) {
		string name;
		int	   days, score, bb, nroles;
		in >> name >> days >> score >> bb >> nroles;

		const ProjectID project_id = PROJECTS.size();
		vec<Role>		roles;
		roles.reserve(nroles);
		umap<SkillID, int> skills;

		for (auto j = 0; j < nroles; ++j) {
			string skill_name;
			int	   skill_level;
			in >> skill_name >> skill_level;

			const int	  role_index = roles.size();
			const SkillID skill		 = SKILL_NAME_TO_ID.at(skill_name);
			roles.push_back(Role{
				.id	   = role_index,
				.skill = skill,
				.level = skill_level,
			});
			skills[skill] = role_index;
		}
		PROJECTS.push_back(Project{
			.id		= project_id,
			.name	= name,
			.bb		= bb,
			.score	= score,
			.days	= days,
			.roles	= roles,
			.skills = skills,
		});
	}
}

static int								   CURRENT_DAY = 0;
static uset<UserID>						   AVAILABLE_USERS;
static uset<ProjectID>					   AVAILABLE_PROJECTS;
static uset<ProjectID>					   PROJECTS_CAN_START;
static std::priority_queue<OngoingProject> ONGOING_PROJECTS;
static umap<SkillID, uset<UserID>>		   AVAILABLE_SKILL_USERS;
static vec<OngoingProject>				   COMPLETED;

bool helper_project_can_start(ProjectID project_id) {
	const auto&	 project = PROJECTS[project_id];
	uset<UserID> used_users;
	for (const auto& role: project.roles) {
		for (const auto& user_id: AVAILABLE_SKILL_USERS[role.skill]) {
			if (used_users.contains(user_id))
				continue;
			const auto& user	   = USERS[user_id];
			const auto	user_level = user.level(role.skill);
			if (user_level >= role.level) {
				used_users.insert(user_id);
				break;
			}
			// else if (user_level == role.level - 1) {
			//     // check already present users
			//	for (const auto mentor_id: used_users) {
			//		const auto& mentor = USERS[mentor_id];
			//		if (mentor.level(role.skill) >= role.level) {
			//			used_users.insert(user_id);
			//			break;
			//		}
			//	}
			// }
		}
	}
	return used_users.size() == project.roles.size();
}

void setup() {
	for (const auto& user: USERS) {
		for (const auto [skill_id, level]: user.skills) {
			AVAILABLE_SKILL_USERS[skill_id].insert(user.id);
		}
		AVAILABLE_USERS.insert(user.id);
	}
	for (const auto& project: PROJECTS) {
		AVAILABLE_PROJECTS.insert(project.id);
		if (helper_project_can_start(project.id))
			PROJECTS_CAN_START.insert(project.id);
	}
}

ProjectID helper_find_best_project() {
	int			   best_remaining = INT_MAX;
	int			   best_project	  = INVALID_ID;
	vec<ProjectID> remove_queue;
	for (const auto project_id: AVAILABLE_PROJECTS) {
		if (!helper_project_can_start(project_id))
			continue;
		const auto& project	  = PROJECTS[project_id];
		const int	remaining = (project.bb - project.days) - CURRENT_DAY;
		if (remaining + project.score <= 0) {
			remove_queue.push_back(project.id);
		} else if (remaining < best_remaining) {
			best_remaining = remaining;
			best_project   = project.id;
		}
	}
	return best_project;
}

OngoingProject helper_make_ongoing(ProjectID project_id) {
	cerr << "MAKING ONGOING " << project_id << endl;
	const auto& project	  = PROJECTS[project_id];
	const int	finishday = CURRENT_DAY + project.days;

	uset<UserID>			  used_users;
	vec<pair<UserID, RoleID>> users;
	users.reserve(project.roles.size());

	for (const auto& role: project.roles) {
		for (const auto user_id: AVAILABLE_SKILL_USERS[role.skill]) {
			if (used_users.contains(user_id))
				continue;

			const auto& user	   = USERS[user_id];
			const auto	user_level = user.level(role.skill);
			if (user_level >= role.level) {
				used_users.insert(user_id);
				users.push_back(std::make_pair(user_id, role.id));
				break;
			}
		}
	}

	return OngoingProject{
		.id		   = project_id,
		.users	   = users,
		.finishday = finishday,
	};
}

void helper_start_ongoing(OngoingProject ongoing) {
	const auto project_id = ongoing.id;

	AVAILABLE_PROJECTS.erase(project_id);
	PROJECTS_CAN_START.erase(project_id);
	for (const auto [user_id, role_id]: ongoing.users) {
		const auto& user = USERS[user_id];
		AVAILABLE_USERS.erase(user_id);
		for (const auto [skill_id, level]: user.skills)
			AVAILABLE_SKILL_USERS[skill_id].erase(user_id);
	}

	ONGOING_PROJECTS.push(std::move(ongoing));
}

void helper_pop_ongoing() {
	assert(!ONGOING_PROJECTS.empty());
	const auto	ongoing = ONGOING_PROJECTS.top();
	const auto& project = PROJECTS[ongoing.id];

	ONGOING_PROJECTS.pop();
	COMPLETED.push_back(ongoing);

	for (const auto [user_id, role_id]: ongoing.users) {
		AVAILABLE_USERS.insert(user_id);

		const auto& role = project.roles[role_id];
		auto&		user = USERS[user_id];
		if (user.level(role.skill) <= role.level) {
			cerr << "RAISING " << SKILL_ID_TO_NAME[role.skill] << " FOR " << user.name << endl;
			user.skills[role.skill]++;
		}

		for (auto& [skill_id, level]: user.skills) {
			AVAILABLE_SKILL_USERS[skill_id].insert(user_id);
		}
	}

	CURRENT_DAY = ongoing.finishday;
	cerr << "FINISHED " << ongoing.id << " AT DAY " << CURRENT_DAY << endl;
}

auto main(int argc, char** argv) -> int {
	auto file = std::fstream(argv[1], std::ios_base::in);
	read_input(file);
	setup();

	while (true) {
		const auto project_id = helper_find_best_project();
		if (project_id == INVALID_ID) {
			if (ONGOING_PROJECTS.empty())
				break;
			helper_pop_ongoing();
		} else {
			const auto ongoing = helper_make_ongoing(project_id);
			helper_start_ongoing(std::move(ongoing));
		}
	}

	cout << COMPLETED.size() << endl;
	for (const auto& completed: COMPLETED) {
		const auto& project = PROJECTS[completed.id];
		cout << project.name << endl;
		for (const auto [user_id, role_id]: completed.users)
			cout << USERS[user_id].name << " ";
		cout << endl;
	}

	return 0;
}
