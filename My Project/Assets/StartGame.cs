using System.Collections;
using UnityEngine.UI;
using UnityEngine;
using UnityEngine.SceneManagement;

public class StartGame : MonoBehaviour
{
    public string LoseScene = "LoseScene";

    public float timeLeft = 3.0f;
    public Text startText; // used for showing countdown from 3, 2, 1

    private void Start()
    {
        AkSoundEngine.PostEvent("BGM", this.gameObject);
        
    }
    

    void Update()
    {
        
        timeLeft -= Time.deltaTime;
        startText.text = (timeLeft).ToString("0");

        if (timeLeft <= 0)
        {
            timeLeft = 0;

            SceneManager.LoadScene(SceneManager.GetActiveScene().buildIndex + 1);

            AkSoundEngine.PostEvent("BGM_Stop", this.gameObject);



        }


        
    }
}
