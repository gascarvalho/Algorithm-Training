import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.StringTokenizer;

public class Main {

    public static void main(String[] args) {
        int R, C;
        int[] b = {1, 2, 3, 4, 6, 8, 10, 12, 16};

        int[] exp;


        try (BufferedReader in = new BufferedReader(new InputStreamReader(System.in))) {
            StringTokenizer stk = new StringTokenizer(in.readLine());

            R = Integer.parseInt(stk.nextToken());
            C = Integer.parseInt(stk.nextToken());

            int max = 0;
            exp = new int[C + 1];

            int seqNr = 0;
            for (int i = 0; i < R; i++) {
                String line = in.readLine();
                char fst = line.charAt(0);
                seqNr = 0;

                for (int j = 0; j < C; j++) {
                    if (j == C - 1) {
                        if (line.charAt(j) == fst) {
                            seqNr++;
                        } else {
                            exp[1]++;
                        }
                        if (fst != '.') exp[seqNr]++;
                        break;
                    }

                    if (line.charAt(j) != fst) {
                        if (fst != '.') exp[seqNr]++;
                        seqNr = 1;
                        fst = line.charAt(j);
                    } else {
                        seqNr++;
                    }

                    max = Math.max(max, seqNr);
                }
            }

            System.out.println(solve(exp, b, max));

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private static long solve(int[] seqNrs, int[] b, int max) {
        long solution = 1;
        int[] results = new int[max + 1];

        for (int i = 0; i <= max; ++i) {
            if (i == 0) {
                results[i] = 1;
            } else {
                for (int size : b) {
                    if (size > i) break;
                    results[i] += results[i - size];
                }
            }
            ´
            solution *= Math.pow(results[i], seqNrs[i]);
        }

        return solution;
    }
}