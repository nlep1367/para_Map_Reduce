
#include "MRUtilities.h"
#include "MRWFiles.h"
#include "FileReader.h"
#include <iomanip>

void show(map<string, unsigned> m_p)
{
    cout << " +" << setfill('-') << setw(22) << "+" << setfill('-') << setw(14) << "+" << endl;
    cout << " | Mot" << setfill(' ') << setw(19) << " | " << "Apparition | " << endl;
    cout << " +" << setfill('-') << setw(22) << "+" << setfill('-') << setw(14) << "+" << endl;
    for_each(m_p.begin(), m_p.end(), [](auto m_pair)
    {
        cout << " | " << setfill(' ') << setw(19) << m_pair.first << " |     " << m_pair.second << setw(9 - to_string(m_pair.second).size()) << " |" << endl;
    });
    cout << " +" << setfill('-') << setw(22) << "+" << setfill('-') << setw(14) << "+" << endl;
}

void showData(map<string, unsigned>& m_p, unsigned nb_mot_traites, ostream &out_stream)
{
    float divs[] = { 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0 };
    unsigned i = 0;
    auto p_mot_it = m_p.begin();
	out_stream << endl;
	out_stream << " | Nombre de mots dans le map : " << m_p.size() << endl;
	out_stream << " | Nombre de mots traites     : " << nb_mot_traites << endl;
	out_stream << " +" << setfill('-') << setw(47) << "+" << endl;
	out_stream << " |     1er mot : " << p_mot_it->first << setfill(' ') << setw(20 - p_mot_it->first.length()) << " = " << setw(5) << p_mot_it->second << " fois +" << endl;
    for_each(divs, divs + 10, [&](float div)
    {
        while (i != int(div * m_p.size()) - 1) { ++i; ++p_mot_it; }
		out_stream << " | " << setfill(' ') << setw(6) << i << "e mot : " << p_mot_it->first << setfill(' ') << setw(20 - p_mot_it->first.length()) << " = " << setw(5) << p_mot_it->second << " fois +" << endl;
    });
	out_stream << " +" << setfill('-') << setw(47) << "+" << endl;
}

void showTestData(unsigned int nb_fichiers, unsigned nb_mot_traites, time_length duration, string test_out_filename, ostream &out_stream)
{
	out_stream << nb_fichiers << " fichiers, " << nb_mot_traites << " mots, " << duration.count() /1000 << " ms." << endl;
	if(&out_stream == &cout) out_stream << "Resultats detailles dans \"" << test_out_filename << "\"" << endl;
}

void showTime(vector<time_length>& v_t, ostream &out_stream)
{
	unsigned i = 0;
	auto end = v_t.end(); --end;
	out_stream << endl;
	out_stream << " |-> Thread_main" << " : " << chrono::duration_cast<time_unit>(*end).count() << setfill(' ') << setw(8) << "  (ms)" << endl;
	if (v_t.size() > 1) out_stream << " +----+" << endl;
	for_each(v_t.begin(), end, [&](time_length& t_s)
	{
		out_stream << "      |-> Thread_" << i << setfill(' ') << setw(4) << " : " << chrono::duration_cast<time_unit>(t_s).count() << setw(8) << "  (ms)" << endl;
		++i;
	});
	if (v_t.size() > 1) reduce(v_t, addVector<time_length>);
	out_stream << " +----+" << setfill('-') << setw(29) << "" << endl;
	out_stream << " | Temps de travail : " << chrono::duration_cast<time_unit>(v_t.front()).count() << " (ms)" << endl;
	out_stream << " +----+" << setfill('-') << setw(29) << "" << endl;
}

void createOutTestFile(string filename, unsigned int nb_fichiers, map<string, unsigned>& m_p, unsigned nb_mot_traite, time_length duration, vector<time_length>& v_t)
{
	ofstream myfile(filename);
	if (myfile.is_open())
	{
		showTestData(nb_fichiers, nb_mot_traite, duration, filename, myfile);
		showData(m_p, nb_mot_traite, myfile);
		showTime(v_t, myfile);
	}
	else cout << "Unable to open file";
}

int runner(mr_w_files &mr_w, unsigned int nb_files)
{
	char c;
	string out_seq, out_para;
	float temps_total;
	//Spanner un test
	// 1) La global metric
	//		On y passe le # de thread (si juste 1 = sequentiel)
	GlobalMetric g_m_parallele(thread::hardware_concurrency());
	map<string, unsigned> m_p_parallele;
	mr_w.start(m_p_parallele, &g_m_parallele);

	// Sequentiel
	
	GlobalMetric g_m_sequentielle(1);
	map<string, unsigned> m_p_sequentielle;
	mr_w.start(m_p_sequentielle, &g_m_sequentielle);

	out_seq = "out-sequentiel.txt";
	cout << "\nSequentielle" << endl;
	createOutTestFile(out_seq, nb_files, m_p_sequentielle, g_m_sequentielle.getNumberWordTreated(), g_m_sequentielle.getDuration(), g_m_sequentielle.getThreadsWorkTime());
	showTestData(nb_files, g_m_sequentielle.getNumberWordTreated(), g_m_sequentielle.getDuration(), out_seq, cout);

	out_para = "out-parallele.txt";
	cout << "\nParallele (sans seuil pour l'instant)" << endl;
	createOutTestFile(out_para, nb_files, m_p_parallele, g_m_parallele.getNumberWordTreated(), g_m_parallele.getDuration(), g_m_parallele.getThreadsWorkTime());
	showTestData(nb_files, g_m_parallele.getNumberWordTreated(), g_m_parallele.getDuration(), out_para, cout);

	temps_total = (g_m_parallele.getDuration() + g_m_sequentielle.getDuration()).count()/ 1000.f;
	
	//Algorithme sequetiel : __% du temps total
	cout << "\nAlgorithme sequentiel :";
	cout << g_m_sequentielle.getDuration().count() / (10.f*temps_total) << "% du temps total" << endl;

	//Algorithme parallèles :
	cout << "\nAlgorithmes paralleles :" << endl;

	// sans seuil sequentiel
	cout << "	sans seuil sequentiel, " << g_m_parallele.getDuration().count() / (10.f*temps_total) << "% du temps total" << endl;

	// seuil séquentiel 32, __% du temps total
	// seuil séquentiel 64, __% du temps total
	// seuil séquentiel 128, __% du temps total
	// seuil séquentiel 256, __% du temps total
	// seuil séquentiel 512, __% du temps total
	// seuil séquentiel 1024, __% du temps total
}

int main(int argc, char* argv[])
{
    char c;
    ifstream f_dtata(argv[1]);
    files_data f_d;

    f_dtata >> f_d.nb_files >> f_d.path;

    f_d.files.resize(f_d.nb_files);
    unsigned i = 0;
    while (f_dtata >> f_d.files[i]) { if (i == (f_d.nb_files - 1)) break; ++i; }
    f_d.files.resize(f_d.nb_files);

    f_dtata.close();
	cout << " | " << f_d.nb_files << " fichiers a traiter charges en memoire..." << endl;

	//S'occupe de l'exec
	// Prend le file data pour faire ses tests
    mr_w_files mr_w(f_d);

	if (string(argv[2]) == "tester") runner(mr_w, f_d.nb_files);
	else 
	{
		//Spanner un test
		// 1) La global metric
		//		On y passe le # de thread (si juste 1 = sequentiel)
		GlobalMetric g_m_parallele(thread::hardware_concurrency());
		map<string, unsigned> m_p_parallele;
		mr_w.start(m_p_parallele, &g_m_parallele);

		GlobalMetric g_m_sequentielle(1);
		map<string, unsigned> m_p_sequentielle;
		mr_w.start(m_p_sequentielle, &g_m_sequentielle);

		cout << "Sequentielle" << endl;
		showData(m_p_sequentielle, g_m_sequentielle.getNumberWordTreated(), cout);

		cout << "Parallele" << endl;
		showData(m_p_parallele, g_m_parallele.getNumberWordTreated(), cout);
	}
    cin >> c;

}
